/**
 Author: Xavier Corbillon
 IMT Atlantique

 This class manage the storage in a file of the head position logs
*/
#pragma once

//internal includes
#include "Quaternion.hpp"

//standard library
#include <string>
#include <fstream>
#include <memory>
#include <chrono>
#include <iostream>
#include <iomanip>

namespace IMT {

class Timestamp
{
public:
  Timestamp(long seconds, long microseconds): m_seconds(seconds), m_microseconds(microseconds) {}
  //Timestamp(const OSVR_TimeValue& time): m_seconds(time.seconds), m_microseconds(time.microseconds) {}
  Timestamp(std::chrono::system_clock::time_point tp): m_seconds(0), m_microseconds(0)
  {
    using namespace std::chrono;
    auto duration = tp.time_since_epoch();
    seconds s = duration_cast<seconds>(duration);
    microseconds m = duration_cast<microseconds>(duration-s);
    m_seconds = s.count();
    m_microseconds = m.count();
  }
  ~Timestamp(void) = default;

  const long& GetSec(void) const {return m_seconds;}
  const long& GetMicrosec(void) const {return m_microseconds;}
  bool operator<(const Timestamp& t) const {return GetSec() < t.GetSec() || (GetSec() == t.GetSec()  && GetMicrosec() < t.GetMicrosec());}
  bool operator<=(const Timestamp& t) const {return GetSec() < t.GetSec() || (GetSec() == t.GetSec()  && GetMicrosec() <= t.GetMicrosec());}
  bool operator>(const Timestamp& t) const {return t < *this;}
  bool operator>=(const Timestamp& t) const {return t <= *this;}
  bool operator==(const Timestamp& t) const {return GetSec() == t.GetSec() && GetMicrosec() == t.GetMicrosec();}
  Timestamp& operator-=(const Timestamp& t)
  {
    m_microseconds -= t.m_microseconds;
    if (m_microseconds < 0)
    {
      m_microseconds += 1000000;
      m_seconds -= 1;
    }
    m_seconds -= t.m_seconds;
    return *this;
  }
  Timestamp operator-(const Timestamp& t) const
  { Timestamp out(*this); return std::move(out-=t); }
  friend std::ostream& operator<< (std::ostream& stream, const Timestamp& timestamp)
  {
    stream << timestamp.GetSec() <<"." << std::setw(6) << std::setfill('0')
      << std::right << timestamp.GetMicrosec();
  }
private:
  long m_seconds;
  long m_microseconds;
};

class Log
{
public:
  Log(Timestamp t, Quaternion q, size_t frameId): m_t(t), m_q(q), m_frameId(frameId) {};

  const Timestamp& GetTimestamp(void) const {return m_t;};
  const Quaternion& GetQuat(void) const {return m_q;};
  friend std::ostream& operator<< (std::ostream& stream, const Log& log)
  {
    stream << log.m_t << " " << log.m_frameId << " " << log.m_q;
  }
  Log operator-(const Timestamp& t) const
  {
    Log out = *this;
    out.m_t -= t;
    return out;
  }
private:
  Timestamp m_t;
  Quaternion m_q;
  size_t m_frameId;
};

class LogWriter
{
public:
  LogWriter(std::string storageFolder, std::string logId): m_storageFolder(storageFolder),
        m_logId(logId), m_isRunning(false), m_testId(0), m_output(nullptr), m_lastTimestamp(0,0),
        m_startTimestamp(0,0), m_firstTimestamp(true) {}
  virtual ~LogWriter(void) {if (m_output != nullptr) {Stop();}};

  void AddLog(const Log& log);
  void Start(void);
  void Stop(void);
private:
  std::string m_storageFolder;
  std::string m_logId;
  bool m_isRunning;
  unsigned m_testId;
  std::shared_ptr<std::ofstream> m_output;
  Timestamp m_lastTimestamp;
  Timestamp m_startTimestamp;
  bool m_firstTimestamp;
};
}