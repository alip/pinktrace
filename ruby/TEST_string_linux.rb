#!/usr/bin/env ruby
# coding: utf-8

require 'test/unit'

$:.insert(0, '.libs')
require 'PinkTrace'

class TestPinkString < Test::Unit::TestCase
  def test_string_decode_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::String.decode 0, 1
    end
  end

  def test_string_encode_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::String.encode 0, 1, 'pink'
    end
  end

  def test_string_encode_unsafe_esrch
    assert_raise Errno::ESRCH do
      PinkTrace::String.encode! 0, 1, 'pink'
    end
  end
end

class TestPinkString
  def test_string_decode
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      File.open '/dev/null'
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid, PinkTrace::Trace::OPTION_SYSGOOD

    # Loop until we get to the open() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if name == 'open' then
          s = PinkTrace::String.decode pid, 0
          assert(s == '/dev/null', "Wrong string, expected: /dev/null got: '#{s}'")
          break
        end
      end
    end

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_string_decode_max
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      File.open '/dev/null'
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid, PinkTrace::Trace::OPTION_SYSGOOD

    # Loop until we get to the open() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if name == 'open' then
          s = PinkTrace::String.decode pid, 0, 10
          assert(s == '/dev/null', "Wrong string, expected: /dev/null got: '#{s}'")
          break
        end
      end
    end

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end

  def test_string_encode
    pid = fork do
      PinkTrace::Trace.me
      Process.kill 'STOP', Process.pid

      begin
        File.open '/dev/null'
      rescue Errno::ENOENT
        exit 0
      end
      exit 1
    end
    Process.waitpid pid
    PinkTrace::Trace.setup pid, PinkTrace::Trace::OPTION_SYSGOOD

    # Loop until we get to the open() system call as there's no guarantee that
    # other system calls won't be called beforehand.
    event = -1
    while event != PinkTrace::Event::EVENT_EXIT_GENUINE
      PinkTrace::Trace.syscall pid
      Process.waitpid pid

      event = PinkTrace::Event.decide
      if event == PinkTrace::Event::EVENT_SYSCALL then
        scno = PinkTrace::Syscall.get_no pid
        name = PinkTrace::Syscall.name scno
        if name == 'open' then
          PinkTrace::String.encode pid, 0, '/dev/NULL'
        end
      end
    end

    assert $?.exited?, "Child hasn't exited!"
    assert($?.exitstatus == 0, "Wrong exit status, expected: 0 got: #{$?.exitstatus}")

    begin PinkTrace::Trace.kill pid
    rescue Errno::ESRCH ;end
  end
end
