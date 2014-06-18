class Greeter
  def initialize(name = "World")
    @name = name
  end
  def nothing
  end
  def say_hi
    puts "Hello, #{@name}!"
#    return nil
  end
  def echo(val)
    puts "#{val}"
  end
  #def call_me
  #end
  def try
    call_me
  end
  def iter
    for i in 0..5
      yield("hello", i)
    end
  end
end
