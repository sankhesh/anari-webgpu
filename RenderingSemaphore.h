// Copyright 2026 The anari-webgpu Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <condition_variable>
#include <mutex>

namespace anari_webgpu {

struct RenderingSemaphore
{
  void arrayMapAcquire()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_frameDone.wait(lock, [&]() { return !m_frameInFlight; });
    m_arraysMapped++;
  }

  void arrayMapRelease()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_arraysMapped > 0 && --m_arraysMapped == 0)
      m_arraysUnmapped.notify_all();
  }

  void frameStart()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_arraysUnmapped.wait(lock, [&]() { return m_arraysMapped == 0; });
    m_frameInFlight = true;
  }

  void frameEnd()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_frameInFlight = false;
    m_frameDone.notify_all();
  }

 private:
  std::mutex m_mutex;
  std::condition_variable m_arraysUnmapped;
  std::condition_variable m_frameDone;
  unsigned long m_arraysMapped{0};
  bool m_frameInFlight{false};
};

} // namespace anari_webgpu
