#!/usr/bin/env ruby
# coding: utf-8

require "fileutils"
require "pathname"
require "shellwords"
require "tmpdir"

$po_dir = File.absolute_path(File.dirname(__FILE__) + "/../../po")

module AddPo
  def self.handle_po file_name
    file_name = File.absolute_path file_name
    content   = IO.
      readlines(file_name).
      map(&:chomp)

    language = content.
      map { |line| /^\" Language: \s+ ([a-z_]+)/ix.match(line) ? $1 : nil }.
      compact.
      first

    fail "Couldn't extract language from #{file_name}" unless language

    target = "#{$po_dir}/#{language}.po"

    File.open(target, "w") { |file| file.puts content.map(&:chomp).join("\n") }
    File.unlink file_name
    File.chmod 0644, target

    puts "#{file_name} → #{target}"
  end

  def self.unpack_p7z file_name
    system "7z x #{Shellwords.escape(file_name)}"
    fail "7z failed for #{file_name}" unless $?.exitstatus == 0
  end

  def self.unpack_rar file_name
    system "unrar x #{Shellwords.escape(file_name)}"
    fail "unrar failed for #{file_name}" unless $?.exitstatus == 0
  end

  def self.unpack_zip file_name
    system "unzip #{Shellwords.escape(file_name)}"
    fail "unzip failed for #{file_name}" unless $?.exitstatus == 0
  end

  def self.unpack_tar file_name
    system "bsdtar xvf #{Shellwords.escape(file_name)}"
    fail "bsdtar failed for #{file_name}" unless $?.exitstatus == 0
  end

  def self.handle_qm file_name
    Dir.mktmpdir do |dir|
      tm_name = Pathname.new(file_name).basename.sub_ext(".tm").to_s
      system "lconvert -i #{Shellwords.escape(file_name)} -o #{Shellwords.escape(tm_name)}"

      fail "lconvert failed for #{file_name}" unless $?.exitstatus == 0

      handle_ts tm_name

      File.unlink file_name
    end
  end

  def self.handle_ts file_name, language = nil
    content   = IO.
      readlines(file_name).
      map(&:chomp)

    language = content.
      map { |line| /<TS [^>]* language="([a-z_@]+)"/ix.match(line) ? $1 : nil }.
      compact.
      first

    if !language
      language = ENV['TS'] if ENV['TS'] && !ENV['TS'].empty?
      language = $1        if !language && /qtbase_([a-zA-Z_@]+)/.match(file_name)
    end

    fail "Unknown language for Qt TS file #{file_name} (set TS)" if !language

    target = "#{$po_dir}/qt/qtbase_#{language}.ts"

    if !FileTest.exists?(target) && /^([a-z]+)_[a-z]+/i.match(language)
      target = "#{$po_dir}/qt/qtbase_#{$1}.ts"
    end

    fail "target file does not exist yet: #{target} (wrong language?)" if !FileTest.exists?(target)

    File.open(target, "w") { |file| file.puts content.map(&:chomp).join("\n") }
    File.unlink file_name
    File.chmod 0644, target
  end

  def self.handle_archive file_name, archive_type
    Dir.mktmpdir do |dir|
      Dir.chdir(dir) do
        send "unpack_#{archive_type}".to_sym, file_name
        Dir["**/*.po"].each { |unpacked_file| handle_po unpacked_file }
        Dir["**/*.qm"].each { |unpacked_file| handle_qm unpacked_file }
        Dir["**/*.ts"].each { |unpacked_file| handle_ts unpacked_file }
      end
    end

    File.unlink file_name
  end

  def self.add file_name
    return handle_archive(file_name, :rar) if /\.rar$/i.match file_name
    return handle_archive(file_name, :zip) if /\.zip$/i.match file_name
    return handle_archive(file_name, :p7z) if /\.7z$/i.match  file_name
    return handle_archive(file_name, :tar) if /\.tar(?:\.(?:gz|bz2|xz))?$/i.match  file_name
    return handle_po(file_name)            if /\.po$/i.match  file_name
    return handle_qm(file_name)            if /\.qm$/i.match  file_name
    return handle_ts(file_name)            if /\.ts$/i.match  file_name
    fail "Don't know how to handle #{file_name}"
  end
end

begin
  # Running under Rake?
  Rake
rescue
  ARGV.each { |file_name| AddPo::add File.absolute_path(file_name) }
end
