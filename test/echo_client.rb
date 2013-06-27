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
    socket.print [message.bytes.count].pack('N') # 32-bit unsigned big-endian
    socket.print message
    puts "deliver: #{message}"
    socket.flush
  end

  def receive
    puts "receiving data"
    len  = socket.recv(4).unpack('N')[0] # 32-bit unsigned, big-endian
    puts "data length: #{len}"
    data = ''
    until data.bytes.count == len do
      data += socket.recv (len - data.bytes.count)
    end
      puts "receive: #{data}"
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

connection.deliver "hello world!"

puts connection.receive
