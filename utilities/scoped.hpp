#ifndef INCLUDE_UTILITIES_SCOPED_HPP_23A89884_8700_4671_B58B_B0CD21119C8E
#define INCLUDE_UTILITIES_SCOPED_HPP_23A89884_8700_4671_B58B_B0CD21119C8E

namespace utilities
{
  template <typename T, typename D>
  class scoped_handle {
  public:
    scoped_handle(T obj, D cln)
      : obj(obj),
	cln(cln),
	own(true)
    {
    }
    scoped_handle(scoped_handle&& other)
      : obj(other.obj),
	cln(other.cln),
	own(true)
    {
      other.own = false;
    }
    ~scoped_handle()
    {
      if (own) {
	cln(obj);
      }
    }

    scoped_handle& operator = (scoped_handle&&) = delete;
    scoped_handle(scoped_handle const&) = delete;
    scoped_handle& operator = (scoped_handle const&) = delete;

    operator T&() {
      return this->obj;
    }
    operator T const&() const {
      return this->obj;
    }

  private:
    T obj;
    D cln;
    bool own;
  };

  template <typename T, typename D>
  inline auto scoped(T obj, D cln) {
    return scoped_handle<T, D>(obj, cln);
  }
}

#endif/*INCLUDE_UTILITIES_SCOPED_HPP_23A89884_8700_4671_B58B_B0CD21119C8E*/
