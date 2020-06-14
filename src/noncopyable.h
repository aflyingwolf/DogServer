
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
namespace server {

class noncopyable {
 protected:
  noncopyable() {}
  ~noncopyable() {}
 private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);   
};


}
