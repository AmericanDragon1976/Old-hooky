#!/usr/bin/env ruby

require 'socket'
require 'oj'
require 'multi_json'

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
    puts "delivering: #{message}"
    socket.print [message.bytes.count].pack('L') # 32-bit unsigned, native endian (uint32_t)
    socket.print message
    socket.flush
    puts "message delivered"
  end

  def receive
    puts "receiving response"
    len = socket.recv(4).unpack('L')[0] # 32-bit unsigned, native endian (uint32_t)
    puts "response length: #{len}"
    data = ''
    until data.bytes.count == len do
      data += socket.recv (len - data.bytes.count)
    end
    puts "response received"
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

request = {
  hook: 'bacon.configure',
  payload: 'lorem nibh, ut pharetra eros lacinia eget. Quisque id arcu nisl. Aliquam bibendum porta suscipit. Maecenas eu tortor'
}

connection.deliver MultiJson.dump(request)

puts connection.receive
