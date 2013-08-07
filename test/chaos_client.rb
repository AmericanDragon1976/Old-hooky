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
    socket.print [message.bytes.count].pack('L') # 32-bit unsigned, native endian (uint32_t)
    socket.print message
    socket.flush
  end

  def receive
    #puts "receive"
    len = socket.recv(4).unpack('L')[0] # 32-bit unsigned, native endian (uint32_t)
    #puts "length: #{len}"
    data = socket.read(len)
    #until data.bytes.count == len do
    #  data += socket.recv (len - data.bytes.count)
      #puts "size: #{data.length} count: #{data.bytes.count}"
    #end
    data
    # socket.recv 0
  end

  def socket
    @socket ||= establish_connection
  end

  alias :connect :socket

  def establish_connection
    _socket = TCPSocket.open @host, @port
    _socket.setsockopt(Socket::IPPROTO_TCP,Socket::TCP_NODELAY, 1)
    _socket
  end
end

host = '0.0.0.0'
port = '4000'

connection = Connection.new host, port

messages = []

%w(one two three four).each do |number|
  messages << File.read(File.expand_path("../ipsum/#{number}.txt", __FILE__))
end

current_iteration = 0

10000.times do

  current_iteration += 1
  
  if current_iteration % 100 == 0
    puts current_iteration 
  end

  #message = messages[rand(4)]

  request = {
    hook: 'sandbox.test',
    payload: "iteration: #{current_iteration}"
  }

  connection.deliver MultiJson.dump(request)

  response = connection.receive

  #puts response

  # puts MultiJson.load(response, symbolize_keys: true)

end