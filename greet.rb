class Greeter
  def initialize(name = "World")
    @name = name
  end
  def nothing
  end
  def say_hi
    puts "Hello, #{@name}!"
  end
  def echo(val)
    puts "#{val}"
  end
  def try
    puts "Call me!"
    call_me
  end
  def iter
    for i in 0..5
      yield("hello", i)
    end
  end
  def big
    return 5234567890987654321
  end
  def set_time (t)
    @time = t
    return
  end
  def say_time
    puts "Year is #{@time.year}"
  end
end

class BenchmarkHelper
  def initialize()
    @val = 0
  end
  def go
    @val = @val + 1
  end
end

def speak
  puts "Hi, Stan!"
end

my_var = 5
