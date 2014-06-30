require "./greet.rb"
require "./test/test.rb"

greet = Greeter.new()

class Child < Greeter
  def call_me
    puts 'Hello, Stan!'
  end
end

h = binding
eval "b = 5", TOPLEVEL_BINDING
puts eval "b", TOPLEVEL_BINDING
