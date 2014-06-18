require "./greet.rb"

greet = Greeter.new()

class Child < Greeter
  def call_me
    puts 'Hello, Stan!'
  end
end

Child.new.try

t = greet.echo('hi')
puts t
