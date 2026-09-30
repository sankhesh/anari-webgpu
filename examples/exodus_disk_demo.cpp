// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#include <anari/anari.h>
#include <netcdf.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void ncCheck(int status, const char *what)
{
  if (status != NC_NOERR) {
    fprintf(stderr, "netCDF error (%s): %s\n", what, nc_strerror(status));
    exit(1);
  }
}

static size_t ncDimLen(int ncid, const char *name)
{
  int dimid = 0;
  ncCheck(nc_inq_dimid(ncid, name, &dimid), name);
  size_t len = 0;
  ncCheck(nc_inq_dimlen(ncid, dimid, &len), name);
  return len;
}

struct ExodusMesh {
  size_t numNodes = 0;
  size_t numElems = 0;
  std::vector<float> positions;
  std::vector<int> connectivity;
  std::vector<float> scalar;
  std::string scalarName;
};

static ExodusMesh readExodusMesh(const std::string &path,
    const std::string &requestedVar)
{
  ExodusMesh mesh;

  int ncid = 0;
  ncCheck(nc_open(path.c_str(), NC_NOWRITE, &ncid), path.c_str());

  mesh.numNodes = ncDimLen(ncid, "num_nodes");
  mesh.numElems = ncDimLen(ncid, "num_el_in_blk1");
  size_t nodPerElem = ncDimLen(ncid, "num_nod_per_el1");
  size_t numDim = ncDimLen(ncid, "num_dim");
  size_t numNodVar = ncDimLen(ncid, "num_nod_var");
  size_t lenString = ncDimLen(ncid, "len_string");

  if (nodPerElem != 8) {
    fprintf(stderr,
        "exodus_disk_demo: only HEX8 element blocks are supported "
        "(got %zu nodes/element)\n",
        nodPerElem);
    exit(1);
  }
  if (numDim != 3) {
    fprintf(stderr, "exodus_disk_demo: only 3-D meshes are supported\n");
    exit(1);
  }

  int coordVar = 0;
  ncCheck(nc_inq_varid(ncid, "coord", &coordVar), "coord");
  std::vector<float> coordSoA(numDim * mesh.numNodes);
  ncCheck(nc_get_var_float(ncid, coordVar, coordSoA.data()), "coord");

  mesh.positions.resize(mesh.numNodes * 3);
  for (size_t i = 0; i < mesh.numNodes; i++) {
    mesh.positions[i * 3 + 0] = coordSoA[0 * mesh.numNodes + i];
    mesh.positions[i * 3 + 1] = coordSoA[1 * mesh.numNodes + i];
    mesh.positions[i * 3 + 2] = coordSoA[2 * mesh.numNodes + i];
  }

  int connVar = 0;
  ncCheck(nc_inq_varid(ncid, "connect1", &connVar), "connect1");
  std::vector<int> conn1Based(mesh.numElems * nodPerElem);
  ncCheck(nc_get_var_int(ncid, connVar, conn1Based.data()), "connect1");

  mesh.connectivity.resize(conn1Based.size());
  for (size_t i = 0; i < conn1Based.size(); i++)
    mesh.connectivity[i] = conn1Based[i] - 1;

  int nameVar = 0;
  ncCheck(nc_inq_varid(ncid, "name_nod_var", &nameVar), "name_nod_var");
  std::vector<char> nameBuf(numNodVar * lenString, '\0');
  ncCheck(nc_get_var_text(ncid, nameVar, nameBuf.data()), "name_nod_var");

  int varIndex = -1;
  std::vector<std::string> names(numNodVar);
  for (size_t v = 0; v < numNodVar; v++) {
    names[v] = std::string(&nameBuf[v * lenString]);
    if (names[v] == requestedVar)
      varIndex = static_cast<int>(v);
  }
  if (varIndex < 0) {
    fprintf(stderr,
        "exodus_disk_demo: nodal variable \"%s\" not found. Available:\n",
        requestedVar.c_str());
    for (auto &n : names)
      fprintf(stderr, "  %s\n", n.c_str());
    exit(1);
  }
  mesh.scalarName = requestedVar;

  int valsVar = 0;
  ncCheck(nc_inq_varid(ncid, "vals_nod_var", &valsVar), "vals_nod_var");
  size_t numTimeSteps = ncDimLen(ncid, "time_step");
  std::vector<float> valsAll(numTimeSteps * numNodVar * mesh.numNodes);
  ncCheck(nc_get_var_float(ncid, valsVar, valsAll.data()), "vals_nod_var");

  mesh.scalar.resize(mesh.numNodes);
  const size_t base = static_cast<size_t>(varIndex) * mesh.numNodes;
  for (size_t i = 0; i < mesh.numNodes; i++)
    mesh.scalar[i] = valsAll[base + i];

  ncCheck(nc_close(ncid), "close");
  return mesh;
}

struct SurfaceMesh {
  std::vector<uint32_t> indices;
  std::vector<float> normals;
};

struct FaceKey {
  std::array<int, 4> sorted;
  bool operator==(const FaceKey &o) const { return sorted == o.sorted; }
};
struct FaceKeyHash {
  size_t operator()(const FaceKey &k) const
  {
    size_t h = 1469598103934665603ull;
    for (int v : k.sorted) {
      h ^= static_cast<size_t>(v);
      h *= 1099511628211ull;
    }
    return h;
  }
};
static FaceKey makeFaceKey(std::array<int, 4> quad)
{
  FaceKey key{quad};
  std::sort(key.sorted.begin(), key.sorted.end());
  return key;
}

static const int kExodusHexSide[6][4] = {
    {0, 1, 5, 4},
    {1, 2, 6, 5},
    {2, 3, 7, 6},
    {0, 4, 7, 3},
    {0, 3, 2, 1},
    {4, 5, 6, 7},
};

static std::vector<std::array<int, 4>> readSideSetQuads(
    const std::string &path, int sideSetId, const ExodusMesh &mesh)
{
  int ncid = 0;
  ncCheck(nc_open(path.c_str(), NC_NOWRITE, &ncid), path.c_str());

  char dimName[32], elemName[32], sideName[32];
  snprintf(dimName, sizeof(dimName), "num_side_ss%d", sideSetId);
  snprintf(elemName, sizeof(elemName), "elem_ss%d", sideSetId);
  snprintf(sideName, sizeof(sideName), "side_ss%d", sideSetId);

  size_t n = ncDimLen(ncid, dimName);
  int elemVar = 0, sideVar = 0;
  ncCheck(nc_inq_varid(ncid, elemName, &elemVar), elemName);
  ncCheck(nc_inq_varid(ncid, sideName, &sideVar), sideName);

  std::vector<int> elems(n), sides(n);
  ncCheck(nc_get_var_int(ncid, elemVar, elems.data()), elemName);
  ncCheck(nc_get_var_int(ncid, sideVar, sides.data()), sideName);
  ncCheck(nc_close(ncid), "close");

  std::vector<std::array<int, 4>> quads;
  quads.reserve(n);
  for (size_t i = 0; i < n; i++) {
    int e = elems[i] - 1;
    int s = sides[i] - 1;
    const int *hex = &mesh.connectivity[static_cast<size_t>(e) * 8];
    const int *ln = kExodusHexSide[s];
    quads.push_back({hex[ln[0]], hex[ln[1]], hex[ln[2]], hex[ln[3]]});
  }
  return quads;
}

static SurfaceMesh triangulateQuads(
    const std::vector<std::array<int, 4>> &quads, const ExodusMesh &mesh)
{
  SurfaceMesh surf;
  surf.indices.reserve(quads.size() * 6);
  surf.normals.assign(mesh.numNodes * 3, 0.f);

  auto addTri = [&](uint32_t a, uint32_t b, uint32_t c) {
    surf.indices.push_back(a);
    surf.indices.push_back(b);
    surf.indices.push_back(c);
    const float *pa = &mesh.positions[a * 3];
    const float *pb = &mesh.positions[b * 3];
    const float *pc = &mesh.positions[c * 3];
    float e1[3] = {pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]};
    float e2[3] = {pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2]};
    float n[3] = {
        e1[1] * e2[2] - e1[2] * e2[1],
        e1[2] * e2[0] - e1[0] * e2[2],
        e1[0] * e2[1] - e1[1] * e2[0],
    };
    for (uint32_t v : {a, b, c})
      for (int k = 0; k < 3; k++)
        surf.normals[v * 3 + k] += n[k];
  };

  for (const auto &q : quads) {
    addTri(q[0], q[1], q[2]);
    addTri(q[0], q[2], q[3]);
  }

  for (size_t i = 0; i < mesh.numNodes; i++) {
    float *n = &surf.normals[i * 3];
    float len = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (len > 1e-12f) {
      n[0] /= len;
      n[1] /= len;
      n[2] /= len;
    }
  }
  return surf;
}

static const int kHexFaces[6][4] = {
    {0, 1, 2, 3},
    {4, 7, 6, 5},
    {0, 4, 5, 1},
    {1, 5, 6, 2},
    {2, 6, 7, 3},
    {3, 7, 4, 0},
};

static SurfaceMesh extractBoundarySurface(const ExodusMesh &mesh,
    const std::unordered_set<FaceKey, FaceKeyHash> &excludeFaces = {})
{
  struct FaceEntry {
    int count = 0;
    std::array<int, 4> winding{};
  };

  std::unordered_map<FaceKey, FaceEntry, FaceKeyHash> faces;
  faces.reserve(mesh.numElems * 6);

  for (size_t e = 0; e < mesh.numElems; e++) {
    const int *hex = &mesh.connectivity[e * 8];
    for (auto &f : kHexFaces) {
      std::array<int, 4> quad = {
          hex[f[0]], hex[f[1]], hex[f[2]], hex[f[3]]};
      FaceKey key = makeFaceKey(quad);

      auto &entry = faces[key];
      entry.count++;
      entry.winding = quad;
    }
  }

  SurfaceMesh surf;
  surf.indices.reserve(faces.size() / 2 * 6);
  surf.normals.assign(mesh.numNodes * 3, 0.f);

  auto addTri = [&](uint32_t a, uint32_t b, uint32_t c) {
    surf.indices.push_back(a);
    surf.indices.push_back(b);
    surf.indices.push_back(c);

    const float *pa = &mesh.positions[a * 3];
    const float *pb = &mesh.positions[b * 3];
    const float *pc = &mesh.positions[c * 3];
    float e1[3] = {pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]};
    float e2[3] = {pc[0] - pa[0], pc[1] - pa[1], pc[2] - pa[2]};
    float n[3] = {
        e1[1] * e2[2] - e1[2] * e2[1],
        e1[2] * e2[0] - e1[0] * e2[2],
        e1[0] * e2[1] - e1[1] * e2[0],
    };
    for (uint32_t v : {a, b, c})
      for (int k = 0; k < 3; k++)
        surf.normals[v * 3 + k] += n[k];
  };

  for (auto &kv : faces) {
    if (kv.second.count != 1)
      continue;
    if (excludeFaces.count(kv.first))
      continue;
    const auto &q = kv.second.winding;
    addTri(q[0], q[1], q[2]);
    addTri(q[0], q[2], q[3]);
  }

  for (size_t i = 0; i < mesh.numNodes; i++) {
    float *n = &surf.normals[i * 3];
    float len = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (len > 1e-12f) {
      n[0] /= len;
      n[1] /= len;
      n[2] /= len;
    }
  }

  return surf;
}

struct IsoMesh {
  std::vector<float> positions;
  std::vector<float> normals;
  std::vector<uint32_t> indices;
};

static const int kHexTetsA[5][4] = {
    {0, 1, 3, 4},
    {1, 2, 3, 6},
    {1, 3, 4, 6},
    {1, 4, 5, 6},
    {3, 4, 6, 7},
};
static const int kHexTetsB[5][4] = {
    {1, 0, 2, 5},
    {0, 3, 2, 7},
    {0, 2, 5, 7},
    {0, 5, 4, 7},
    {2, 5, 6, 7},
};

static IsoMesh extractIsosurface(const ExodusMesh &mesh, float isovalue)
{
  struct QKey {
    int64_t x, y, z;
    bool operator==(const QKey &o) const
    {
      return x == o.x && y == o.y && z == o.z;
    }
  };
  struct QKeyHash {
    size_t operator()(const QKey &k) const
    {
      size_t h = 1469598103934665603ull;
      for (int64_t v : {k.x, k.y, k.z}) {
        h ^= static_cast<size_t>(v);
        h *= 1099511628211ull;
      }
      return h;
    }
  };

  IsoMesh iso;
  std::unordered_map<QKey, uint32_t, QKeyHash> vertexCache;

  auto getOrAddVertex = [&](const float p[3]) -> uint32_t {
    QKey key{std::llround(p[0] * 1e4), std::llround(p[1] * 1e4),
        std::llround(p[2] * 1e4)};
    auto it = vertexCache.find(key);
    if (it != vertexCache.end())
      return it->second;
    uint32_t idx = static_cast<uint32_t>(iso.positions.size() / 3);
    iso.positions.push_back(p[0]);
    iso.positions.push_back(p[1]);
    iso.positions.push_back(p[2]);
    iso.normals.push_back(0.f);
    iso.normals.push_back(0.f);
    iso.normals.push_back(0.f);
    vertexCache.emplace(key, idx);
    return idx;
  };

  auto addTri = [&](const float p0[3], const float p1[3], const float p2[3]) {
    uint32_t i0 = getOrAddVertex(p0);
    uint32_t i1 = getOrAddVertex(p1);
    uint32_t i2 = getOrAddVertex(p2);
    iso.indices.push_back(i0);
    iso.indices.push_back(i1);
    iso.indices.push_back(i2);

    const float *a = &iso.positions[i0 * 3];
    const float *b = &iso.positions[i1 * 3];
    const float *c = &iso.positions[i2 * 3];
    float e1[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    float e2[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
    float n[3] = {
        e1[1] * e2[2] - e1[2] * e2[1],
        e1[2] * e2[0] - e1[0] * e2[2],
        e1[0] * e2[1] - e1[1] * e2[0],
    };
    for (uint32_t v : {i0, i1, i2})
      for (int k = 0; k < 3; k++)
        iso.normals[v * 3 + k] += n[k];
  };

  int hexNode[8];
  auto interp = [&](int a, int b, float out[3]) {
    float sa = mesh.scalar[hexNode[a]], sb = mesh.scalar[hexNode[b]];
    float t = (isovalue - sa) / (sb - sa);
    const float *pa = &mesh.positions[hexNode[a] * 3];
    const float *pb = &mesh.positions[hexNode[b] * 3];
    for (int k = 0; k < 3; k++)
      out[k] = pa[k] + t * (pb[k] - pa[k]);
  };

  for (size_t e = 0; e < mesh.numElems; e++) {
    const int *hex = &mesh.connectivity[e * 8];
    for (int k = 0; k < 8; k++)
      hexNode[k] = hex[k];

    const int (*kHexTets)[4] = (e % 2 == 0) ? kHexTetsA : kHexTetsB;
    for (int t = 0; t < 5; t++) {
      const int *tet = kHexTets[t];
      int mask = 0;
      for (int i = 0; i < 4; i++)
        if (mesh.scalar[hexNode[tet[i]]] >= isovalue)
          mask |= (1 << i);
      if (mask == 0 || mask == 0xF)
        continue;

      int bitCount = ((mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1)
          + ((mask >> 3) & 1));

      if (bitCount == 1 || bitCount == 3) {
        bool singleIsHot = (bitCount == 1);
        int odd = -1;
        for (int i = 0; i < 4; i++) {
          bool hot = (mask >> i) & 1;
          if (hot == singleIsHot) {
            odd = i;
            break;
          }
        }
        int o[3];
        for (int i = 0, j = 0; i < 4; i++)
          if (i != odd)
            o[j++] = i;

        float p0[3], p1[3], p2[3];
        interp(tet[odd], tet[o[0]], p0);
        interp(tet[odd], tet[o[1]], p1);
        interp(tet[odd], tet[o[2]], p2);
        if (singleIsHot)
          addTri(p0, p2, p1);
        else
          addTri(p0, p1, p2);
      } else {
        int hotV[2], coldV[2], hi = 0, ci = 0;
        for (int i = 0; i < 4; i++) {
          if ((mask >> i) & 1)
            hotV[hi++] = i;
          else
            coldV[ci++] = i;
        }
        float q00[3], q01[3], q11[3], q10[3];
        interp(tet[hotV[0]], tet[coldV[0]], q00);
        interp(tet[hotV[0]], tet[coldV[1]], q01);
        interp(tet[hotV[1]], tet[coldV[1]], q11);
        interp(tet[hotV[1]], tet[coldV[0]], q10);
        addTri(q00, q11, q01);
        addTri(q00, q10, q11);
      }
    }
  }

  for (size_t i = 0; i < iso.positions.size() / 3; i++) {
    float *n = &iso.normals[i * 3];
    float len = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (len > 1e-12f) {
      n[0] /= len;
      n[1] /= len;
      n[2] /= len;
    }
  }
  return iso;
}

static void colormap(float t, float rgb[3])
{
  static const float stops[8][3] = {
      {0.10f, 0.15f, 0.45f},
      {0.15f, 0.35f, 0.80f},
      {0.20f, 0.65f, 0.85f},
      {0.55f, 0.85f, 0.60f},
      {0.95f, 0.90f, 0.40f},
      {0.98f, 0.60f, 0.20f},
      {0.90f, 0.25f, 0.15f},
      {0.98f, 0.95f, 0.90f},
  };
  t = std::clamp(t, 0.f, 1.f);
  float scaled = t * 7.f;
  int i0 = static_cast<int>(scaled);
  int i1 = std::min(i0 + 1, 7);
  float frac = scaled - static_cast<float>(i0);
  for (int k = 0; k < 3; k++)
    rgb[k] = stops[i0][k] * (1.f - frac) + stops[i1][k] * frac;
}

static void statusFunc(const void *, ANARIDevice, ANARIObject, ANARIDataType,
    ANARIStatusSeverity severity, ANARIStatusCode, const char *message)
{
  if (severity <= ANARI_SEVERITY_WARNING)
    fprintf(stderr, "[anari] %s\n", message);
}

static void savePPM(
    const char *filename, const uint8_t *pixels, int width, int height)
{
  FILE *f = fopen(filename, "wb");
  if (!f)
    return;
  fprintf(f, "P6\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      const uint8_t *p = pixels + (static_cast<size_t>(y) * width + x) * 4;
      fwrite(p, 1, 3, f);
    }
  }
  fclose(f);
}

enum class ViewTarget { Domain, Disk, Dome };

struct View {
  const char *name;
  float offsetDir[3];
  float distance;
  float fovyDeg;
  ViewTarget target;
};

int main(int argc, char **argv)
{
  std::string path = argc > 1 ? argv[1]
      : std::string(std::getenv("HOME") ? std::getenv("HOME") : "")
          + "/Data/SampleData/disk_out_ref.ex2";
  std::string varName = argc > 2 ? argv[2] : "Temp";
  bool hasIsovalueArg = argc > 3;
  float isovalueArg = hasIsovalueArg ? std::strtof(argv[3], nullptr) : 0.f;

  printf("=== Exodus II Disk Demo ===\n\n");
  printf("Loading %s (variable: %s)...\n", path.c_str(), varName.c_str());
  ExodusMesh mesh = readExodusMesh(path, varName);
  printf("  %zu nodes, %zu HEX8 elements\n", mesh.numNodes, mesh.numElems);

  float scalarMin = mesh.scalar[0], scalarMax = mesh.scalar[0];
  for (float v : mesh.scalar) {
    scalarMin = std::min(scalarMin, v);
    scalarMax = std::max(scalarMax, v);
  }
  printf("  %s range: [%.4f, %.4f]\n", varName.c_str(), scalarMin, scalarMax);

  float isovalue =
      hasIsovalueArg ? isovalueArg : scalarMin + 0.25f * (scalarMax - scalarMin);
  printf("Extracting %s isosurface at %.2f...\n", varName.c_str(), isovalue);
  IsoMesh isoSurf = extractIsosurface(mesh, isovalue);
  printf("  %zu isosurface triangles (%zu vertices)\n",
      isoSurf.indices.size() / 3, isoSurf.positions.size() / 3);

  float isoCenter[3] = {0.f, 0.f, 0.f};
  float isoRadius = 1.f;
  if (!isoSurf.indices.empty()) {
    float imin[3] = {1e30f, 1e30f, 1e30f}, imax[3] = {-1e30f, -1e30f, -1e30f};
    for (size_t i = 0; i < isoSurf.positions.size() / 3; i++) {
      for (int k = 0; k < 3; k++) {
        float p = isoSurf.positions[i * 3 + k];
        imin[k] = std::min(imin[k], p);
        imax[k] = std::max(imax[k], p);
      }
    }
    for (int k = 0; k < 3; k++)
      isoCenter[k] = 0.5f * (imin[k] + imax[k]);
    float iext[3] = {imax[0] - imin[0], imax[1] - imin[1], imax[2] - imin[2]};
    isoRadius = std::max(
        0.5f * std::sqrt(iext[0]*iext[0] + iext[1]*iext[1] + iext[2]*iext[2]),
        0.5f);
    printf("  dome center: (%.3f, %.3f, %.3f), radius: %.3f\n", isoCenter[0],
        isoCenter[1], isoCenter[2], isoRadius);
  } else {
    fprintf(stderr,
        "  warning: isovalue %.2f produced no surface — try a value between "
        "%.2f and %.2f\n",
        isovalue, scalarMin, scalarMax);
  }

  printf("Reading disk surface (side sets 2 + 7)...\n");
  std::vector<std::array<int, 4>> diskQuads = readSideSetQuads(path, 7, mesh);
  {
    auto rim = readSideSetQuads(path, 2, mesh);
    diskQuads.insert(diskQuads.end(), rim.begin(), rim.end());
  }
  printf("  %zu disk quads\n", diskQuads.size());

  std::unordered_set<FaceKey, FaceKeyHash> diskFaceKeys;
  for (auto &q : diskQuads)
    diskFaceKeys.insert(makeFaceKey(q));
  SurfaceMesh diskSurf = triangulateQuads(diskQuads, mesh);

  printf("Extracting remaining domain wall...\n");
  SurfaceMesh wallSurf = extractBoundarySurface(mesh, diskFaceKeys);
  printf("  %zu wall triangles, %zu disk triangles\n",
      wallSurf.indices.size() / 3, diskSurf.indices.size() / 3);

  float diskCenter[3] = {0.f, 0.f, 0.f};
  float diskRadius = 1.f;
  {
    float dmin[3] = {1e30f, 1e30f, 1e30f}, dmax[3] = {-1e30f, -1e30f, -1e30f};
    for (uint32_t idx : diskSurf.indices) {
      for (int k = 0; k < 3; k++) {
        float p = mesh.positions[idx * 3 + k];
        dmin[k] = std::min(dmin[k], p);
        dmax[k] = std::max(dmax[k], p);
      }
    }
    for (int k = 0; k < 3; k++)
      diskCenter[k] = 0.5f * (dmin[k] + dmax[k]);
    float dext[3] = {dmax[0] - dmin[0], dmax[1] - dmin[1], dmax[2] - dmin[2]};
    diskRadius = std::max(
        0.5f * std::sqrt(dext[0]*dext[0] + dext[1]*dext[1] + dext[2]*dext[2]),
        0.5f);
    printf("  disk center: (%.3f, %.3f, %.3f), radius: %.3f\n", diskCenter[0],
        diskCenter[1], diskCenter[2], diskRadius);
  }
  {
    float dsmin = 1e30f, dsmax = -1e30f, wsmin = 1e30f, wsmax = -1e30f;
    for (uint32_t idx : diskSurf.indices) {
      dsmin = std::min(dsmin, mesh.scalar[idx]);
      dsmax = std::max(dsmax, mesh.scalar[idx]);
    }
    for (uint32_t idx : wallSurf.indices) {
      wsmin = std::min(wsmin, mesh.scalar[idx]);
      wsmax = std::max(wsmax, mesh.scalar[idx]);
    }
    printf("  disk %s range: [%.2f, %.2f] (uniform boundary condition)\n",
        varName.c_str(), dsmin, dsmax);
    printf("  wall %s range: [%.2f, %.2f] (scalar-colored)\n", varName.c_str(),
        wsmin, wsmax);
  }

  std::vector<float> colors(mesh.numNodes * 4);
  float range = std::max(scalarMax - scalarMin, 1e-6f);
  for (size_t i = 0; i < mesh.numNodes; i++) {
    float t = (mesh.scalar[i] - scalarMin) / range;
    float rgb[3];
    colormap(t, rgb);
    colors[i * 4 + 0] = rgb[0];
    colors[i * 4 + 1] = rgb[1];
    colors[i * 4 + 2] = rgb[2];
    colors[i * 4 + 3] = 1.f;
  }

  float bmin[3] = {mesh.positions[0], mesh.positions[1], mesh.positions[2]};
  float bmax[3] = {bmin[0], bmin[1], bmin[2]};
  for (size_t i = 0; i < mesh.numNodes; i++) {
    for (int k = 0; k < 3; k++) {
      bmin[k] = std::min(bmin[k], mesh.positions[i * 3 + k]);
      bmax[k] = std::max(bmax[k], mesh.positions[i * 3 + k]);
    }
  }
  float center[3] = {
      0.5f * (bmin[0] + bmax[0]),
      0.5f * (bmin[1] + bmax[1]),
      0.5f * (bmin[2] + bmax[2]),
  };
  float ext[3] = {bmax[0] - bmin[0], bmax[1] - bmin[1], bmax[2] - bmin[2]};
  float radius =
      0.5f * std::sqrt(ext[0] * ext[0] + ext[1] * ext[1] + ext[2] * ext[2]);
  printf("  bounds: [%.3f %.3f %.3f] - [%.3f %.3f %.3f]\n", bmin[0], bmin[1],
      bmin[2], bmax[0], bmax[1], bmax[2]);

  printf("Creating ANARI device...\n");
  ANARILibrary lib = anariLoadLibrary("webgpu", statusFunc, nullptr);
  if (!lib) {
    fprintf(stderr, "Failed to load webgpu library\n");
    return 1;
  }
  ANARIDevice device = anariNewDevice(lib, "default");
  if (!device) {
    fprintf(stderr, "Failed to create device\n");
    anariUnloadLibrary(lib);
    return 1;
  }

  ANARIArray1D posArray = anariNewArray1D(device, mesh.positions.data(), 0, 0,
      ANARI_FLOAT32_VEC3, mesh.numNodes);
  ANARIArray1D colorArray = anariNewArray1D(
      device, colors.data(), 0, 0, ANARI_FLOAT32_VEC4, mesh.numNodes);

  ANARIArray1D diskNormArray = anariNewArray1D(
      device, diskSurf.normals.data(), 0, 0, ANARI_FLOAT32_VEC3, mesh.numNodes);
  ANARIArray1D diskIdxArray = anariNewArray1D(device, diskSurf.indices.data(),
      0, 0, ANARI_UINT32_VEC3, diskSurf.indices.size() / 3);

  ANARIGeometry diskGeom = anariNewGeometry(device, "triangle");
  anariSetParameter(device, diskGeom, "vertex.position", ANARI_ARRAY1D, &posArray);
  anariSetParameter(device, diskGeom, "vertex.normal", ANARI_ARRAY1D, &diskNormArray);
  anariSetParameter(device, diskGeom, "primitive.index", ANARI_ARRAY1D, &diskIdxArray);
  anariCommitParameters(device, diskGeom);

  ANARIMaterial diskMat = anariNewMaterial(device, "matte");
  float diskColor[] = {0.72f, 0.70f, 0.66f, 1.f};
  anariSetParameter(device, diskMat, "color", ANARI_FLOAT32_VEC4, diskColor);
  anariCommitParameters(device, diskMat);

  ANARISurface diskSurface = anariNewSurface(device);
  anariSetParameter(device, diskSurface, "geometry", ANARI_GEOMETRY, &diskGeom);
  anariSetParameter(device, diskSurface, "material", ANARI_MATERIAL, &diskMat);
  anariCommitParameters(device, diskSurface);

  ANARIArray1D wallNormArray = anariNewArray1D(
      device, wallSurf.normals.data(), 0, 0, ANARI_FLOAT32_VEC3, mesh.numNodes);
  ANARIArray1D wallIdxArray = anariNewArray1D(device, wallSurf.indices.data(),
      0, 0, ANARI_UINT32_VEC3, wallSurf.indices.size() / 3);

  ANARIGeometry wallGeom = anariNewGeometry(device, "triangle");
  anariSetParameter(device, wallGeom, "vertex.position", ANARI_ARRAY1D, &posArray);
  anariSetParameter(device, wallGeom, "vertex.normal", ANARI_ARRAY1D, &wallNormArray);
  anariSetParameter(device, wallGeom, "vertex.color", ANARI_ARRAY1D, &colorArray);
  anariSetParameter(device, wallGeom, "primitive.index", ANARI_ARRAY1D, &wallIdxArray);
  anariCommitParameters(device, wallGeom);

  ANARIMaterial wallMat = anariNewMaterial(device, "matte");
  float white[] = {1.f, 1.f, 1.f, 1.f};
  anariSetParameter(device, wallMat, "color", ANARI_FLOAT32_VEC4, white);
  anariCommitParameters(device, wallMat);

  ANARISurface wallSurface = anariNewSurface(device);
  anariSetParameter(device, wallSurface, "geometry", ANARI_GEOMETRY, &wallGeom);
  anariSetParameter(device, wallSurface, "material", ANARI_MATERIAL, &wallMat);
  anariCommitParameters(device, wallSurface);

  ANARISurface isoSurface = nullptr;
  ANARIArray1D isoPosArray = nullptr, isoNormArray = nullptr, isoIdxArray = nullptr;
  ANARIGeometry isoGeom = nullptr;
  ANARIMaterial isoMat = nullptr;
  if (!isoSurf.indices.empty()) {
    size_t isoNumVerts = isoSurf.positions.size() / 3;
    isoPosArray = anariNewArray1D(
        device, isoSurf.positions.data(), 0, 0, ANARI_FLOAT32_VEC3, isoNumVerts);
    isoNormArray = anariNewArray1D(
        device, isoSurf.normals.data(), 0, 0, ANARI_FLOAT32_VEC3, isoNumVerts);
    isoIdxArray = anariNewArray1D(device, isoSurf.indices.data(), 0, 0,
        ANARI_UINT32_VEC3, isoSurf.indices.size() / 3);

    isoGeom = anariNewGeometry(device, "triangle");
    anariSetParameter(device, isoGeom, "vertex.position", ANARI_ARRAY1D, &isoPosArray);
    anariSetParameter(device, isoGeom, "vertex.normal", ANARI_ARRAY1D, &isoNormArray);
    anariSetParameter(device, isoGeom, "primitive.index", ANARI_ARRAY1D, &isoIdxArray);
    anariCommitParameters(device, isoGeom);

    isoMat = anariNewMaterial(device, "physicallyBased");
    float isoBase[] = {0.85f, 0.35f, 0.12f};
    float isoEmissive[] = {0.85f, 0.35f, 0.12f};
    float isoMetallic = 0.f, isoRoughness = 0.55f, isoEmissiveStrength = 0.35f;
    anariSetParameter(device, isoMat, "baseColor", ANARI_FLOAT32_VEC3, isoBase);
    anariSetParameter(device, isoMat, "metallic", ANARI_FLOAT32, &isoMetallic);
    anariSetParameter(device, isoMat, "roughness", ANARI_FLOAT32, &isoRoughness);
    anariSetParameter(device, isoMat, "emissive", ANARI_FLOAT32_VEC3, isoEmissive);
    anariSetParameter(
        device, isoMat, "emissiveStrength", ANARI_FLOAT32, &isoEmissiveStrength);
    anariCommitParameters(device, isoMat);

    isoSurface = anariNewSurface(device);
    anariSetParameter(device, isoSurface, "geometry", ANARI_GEOMETRY, &isoGeom);
    anariSetParameter(device, isoSurface, "material", ANARI_MATERIAL, &isoMat);
    anariCommitParameters(device, isoSurface);
  }

  ANARIWorld world = anariNewWorld(device);
  std::vector<ANARISurface> surfaces = {diskSurface, wallSurface};
  if (isoSurface)
    surfaces.push_back(isoSurface);
  ANARIArray1D surfArray = anariNewArray1D(
      device, surfaces.data(), 0, 0, ANARI_SURFACE, surfaces.size());
  anariSetParameter(device, world, "surface", ANARI_ARRAY1D, &surfArray);

  ANARILight keyLight = anariNewLight(device, "directional");
  float keyDir[] = {-0.4f, -0.8f, -0.5f};
  float keyColor[] = {1.f, 0.98f, 0.95f};
  float keyIrr = 2.2f;
  anariSetParameter(device, keyLight, "direction", ANARI_FLOAT32_VEC3, keyDir);
  anariSetParameter(device, keyLight, "color", ANARI_FLOAT32_VEC3, keyColor);
  anariSetParameter(device, keyLight, "irradiance", ANARI_FLOAT32, &keyIrr);
  anariCommitParameters(device, keyLight);

  ANARILight fillLight = anariNewLight(device, "directional");
  float fillDir[] = {0.6f, 0.2f, 0.7f};
  float fillColor[] = {0.7f, 0.8f, 1.f};
  float fillIrr = 0.6f;
  anariSetParameter(device, fillLight, "direction", ANARI_FLOAT32_VEC3, fillDir);
  anariSetParameter(device, fillLight, "color", ANARI_FLOAT32_VEC3, fillColor);
  anariSetParameter(device, fillLight, "irradiance", ANARI_FLOAT32, &fillIrr);
  anariCommitParameters(device, fillLight);

  ANARILight lights[] = {keyLight, fillLight};
  ANARIArray1D lightArray =
      anariNewArray1D(device, lights, 0, 0, ANARI_LIGHT, 2);
  anariSetParameter(device, world, "light", ANARI_ARRAY1D, &lightArray);
  anariCommitParameters(device, world);

  ANARIRenderer renderer = anariNewRenderer(device, "default");
  float bg[] = {0.05f, 0.05f, 0.08f, 1.f};
  float ambient = 0.15f;
  anariSetParameter(device, renderer, "background", ANARI_FLOAT32_VEC4, bg);
  anariSetParameter(device, renderer, "ambientRadiance", ANARI_FLOAT32, &ambient);
  anariCommitParameters(device, renderer);

  const int width = 1920, height = 1080;
  const View views[] = {
      {"domain_overview", {0.742f, 0.423f, 0.520f}, radius * 2.6f, 45.f,
          ViewTarget::Domain},
      {"disk_face_on", {0.f, 0.f, 1.f}, 8.f, 70.f, ViewTarget::Disk},
      {"disk_wide", {0.366f, 0.211f, 0.906f}, 8.5f, 65.f, ViewTarget::Disk},
      {"disk_closeup", {0.366f, 0.211f, 0.906f}, 5.5f, 55.f, ViewTarget::Disk},
      {"temp_dome", {0.4f, 0.28f, 0.87f}, 9.5f, 65.f, ViewTarget::Dome},
      {"temp_dome_closeup", {0.4f, 0.28f, 0.87f}, 6.5f, 55.f, ViewTarget::Dome},
  };

  ANARIFrame frame = anariNewFrame(device);
  uint32_t size[] = {(uint32_t)width, (uint32_t)height};
  ANARIDataType colorType = ANARI_UFIXED8_VEC4;
  anariSetParameter(device, frame, "size", ANARI_UINT32_VEC2, size);
  anariSetParameter(device, frame, "channel.color", ANARI_DATA_TYPE, &colorType);
  anariSetParameter(device, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(device, frame, "world", ANARI_WORLD, &world);

  for (const auto &v : views) {
    if (v.target == ViewTarget::Dome && isoSurf.indices.empty())
      continue;
    const float *target = v.target == ViewTarget::Disk ? diskCenter
        : v.target == ViewTarget::Dome                 ? isoCenter
                                                         : center;

    float camPos[3] = {
        target[0] + v.distance * v.offsetDir[0],
        target[1] + v.distance * v.offsetDir[1],
        target[2] + v.distance * v.offsetDir[2],
    };
    float camDir[3] = {-v.offsetDir[0], -v.offsetDir[1], -v.offsetDir[2]};

    ANARICamera camera = anariNewCamera(device, "perspective");
    float up[] = {0.f, 1.f, 0.f};
    float fovy = v.fovyDeg * (float)M_PI / 180.f;
    float aspect = (float)width / height;
    anariSetParameter(device, camera, "position", ANARI_FLOAT32_VEC3, camPos);
    anariSetParameter(device, camera, "direction", ANARI_FLOAT32_VEC3, camDir);
    anariSetParameter(device, camera, "up", ANARI_FLOAT32_VEC3, up);
    anariSetParameter(device, camera, "fovy", ANARI_FLOAT32, &fovy);
    anariSetParameter(device, camera, "aspect", ANARI_FLOAT32, &aspect);
    anariCommitParameters(device, camera);

    anariSetParameter(device, frame, "camera", ANARI_CAMERA, &camera);
    anariCommitParameters(device, frame);

    printf("Rendering view \"%s\" (%dx%d)...\n", v.name, width, height);
    anariRenderFrame(device, frame);
    anariFrameReady(device, frame, ANARI_WAIT);

    uint32_t fbW, fbH;
    ANARIDataType fbType;
    const void *pixels =
        anariMapFrame(device, frame, "channel.color", &fbW, &fbH, &fbType);
    if (pixels) {
      std::string outName = std::string("disk_out_ref_") + v.name + ".ppm";
      savePPM(outName.c_str(), (const uint8_t *)pixels, fbW, fbH);
      printf("  Saved: %s\n", outName.c_str());
      anariUnmapFrame(device, frame, "channel.color");
    }

    anariRelease(device, camera);
  }

  anariRelease(device, frame);
  anariRelease(device, renderer);
  anariRelease(device, world);
  anariRelease(device, lightArray);
  anariRelease(device, keyLight);
  anariRelease(device, fillLight);
  anariRelease(device, surfArray);
  anariRelease(device, diskSurface);
  anariRelease(device, diskMat);
  anariRelease(device, diskGeom);
  anariRelease(device, wallSurface);
  anariRelease(device, wallMat);
  anariRelease(device, wallGeom);
  anariRelease(device, posArray);
  anariRelease(device, colorArray);
  anariRelease(device, diskNormArray);
  anariRelease(device, diskIdxArray);
  anariRelease(device, wallNormArray);
  anariRelease(device, wallIdxArray);
  anariRelease(device, isoSurface);
  anariRelease(device, isoMat);
  anariRelease(device, isoGeom);
  anariRelease(device, isoPosArray);
  anariRelease(device, isoNormArray);
  anariRelease(device, isoIdxArray);
  anariRelease(device, device);
  anariUnloadLibrary(lib);

  printf("\n=== Done ===\n");
  return 0;
}
