#pragma once

class PropertiesReceiver {
 public:
  virtual ~PropertiesReceiver() {}

  virtual void add(const std::string& name, const std::string& value,
                   const std::string& description) = 0;
  virtual void clear() = 0;
};
