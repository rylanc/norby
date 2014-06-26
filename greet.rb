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
end

class BenchmarkHelper
  def initialize()
    @val = 0
  end
  def go
    @val = @val + 1
  end
end
