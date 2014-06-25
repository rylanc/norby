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
end
