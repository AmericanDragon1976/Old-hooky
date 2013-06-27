#!/usr/bin/env ruby

require 'socket'

class Connection

  MAX_ATTEMPTS = 30

  attr_accessor :socket
  
  def initialize(host='127.0.0.1', port=3318, autoconnect=true)
    @host     = host
    @port     = port
    @attempts = 0
    connect if autoconnect
  end

  def deliver(message)
    socket.print [message.bytes.count].pack('L') # 32-bit unsigned, native endian (uint32_t)
    socket.print message
    socket.flush
  end

  def receive
    puts "receive"
    len = socket.recv(4).unpack('L')[0] # 32-bit unsigned, native endian (uint32_t)
    puts "length: #{len}"
    data = ''
    until data.bytes.count == len do
      data += socket.recv (len - data.bytes.count)
      puts data
    end
    data
  end

  def socket
    @socket ||= establish_connection
  end

  alias :connect :socket

  def establish_connection
    TCPSocket.open @host, @port
  end
end

host = '127.0.0.1'
port = '4000'

connection = Connection.new host, port

messages = []

%w(one two three four).each do |number|
  messages << File.read(File.expand_path("../ipsum/#{number}.txt", __FILE__))
end

current_iteration = 0
1.times do

  current_iteration += 1
  puts current_iteration

  message = messages[rand(4)]
  connection.deliver message
  result = connection.receive

  if not result == message
    puts "whoa! iteration #{current_iteration} mismatch..."
    puts "original length: #{message.length}"
    puts "received length: #{result.length}"
    exit
  end

end
