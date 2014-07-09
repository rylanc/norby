class Returner
  def ret_nothing
  end
  def ret_nil
    return nil
  end
  def ret_float
    return 4.359
  end
  def ret_fixnum
    return 49895
  end
  def ret_max_int32
    return 2147483647
  end
  def ret_max_precise_double
    return 1 << 53
  end
  def ret_min_int32
    return -2147483648
  end
  def ret_min_precise_double
    return -(1 << 53)
  end
  def ret_bignum
    5234567890987654321
  end
  def ret_true
    return true
  end
  def ret_false
    return false
  end
  def ret_string
    return "hello"
  end
  def ret_array
    return [4, 3, 9]
  end
  def ret_time
    return Time.new(2001, 2, 3)
  end
  def set_time (t)
    @time = t
  end
  def get_time
    return @time
  end
end

class ArgsInspector
  def arg_count (*args)
    return args.length
  end
  def get_type (arg)
    return arg.class.to_s
  end
  def check_nil (arg)
    return arg == nil
  end
  def check_true (arg)
    return arg == true
  end
  def check_false (arg)
    return arg == false
  end
  def check_string (arg)
    return arg == "hello"
  end
  def check_array (arg)
    return arg == [4, 3, 9]
  end
  def check_int32 (arg)
    return arg == -49895
  end
  def check_uint32 (arg)
    return arg == 49895
  end
  def check_number (arg)
    return arg == 4.359
  end
  def check_time (arg)
    return (arg.year == 2001 and arg.month == 2 and arg.mday == 3)
  end
end

class ClassTester
  MY_CONST = [1, 2]
  def initialize(val)
    @val = val
  end
  def get_val
    return @val
  end
end

class BlockTester
  def yield_one
    yield "hello"
  end
  def yield_two
    yield "hello", 2
  end
  def iter
    for i in 0..5
      yield("hi", i)
    end
  end
  def echo (arg)
    yield arg
  end
end

class InheritTester
  def call_derived
  end
  def call_derived_with_args(arg1, arg2)
  end
  def call_derived_with_this
  end
  
  def make_call
    return call_derived
  end
  def make_missing_call
    return call_missing
  end
  def make_call_with_this
    return call_derived_with_this
  end
  def make_call_with_args(arg1, arg2)
    return call_derived_with_args(arg1, arg2)
  end
  def make_invalid_call
    return doesnt_exist
  end
  def valid_responds
    return respond_to? :call_derived
  end
  def invalid_responds
    return respond_to? :doesnt_exist
  end
end

class ArgsInheritTester
  def initialize(arg1, arg2, arg3)
    @arg1 = arg1
    @arg2 = arg2
    @arg3 = arg3
  end
  def get_arg1
    @arg1
  end
  def get_arg2
    @arg2
  end
  def get_arg3
    @arg3
  end
end

def non_class_function (arg1, arg2, arg3)
  return arg1 + arg2 + arg3
end

module MyMod
  MY_CONST = "abcde"
  class ModClass
    def call_me
      return 3.14159
    end
  end
end
