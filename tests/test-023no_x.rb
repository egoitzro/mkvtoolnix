#!/usr/bin/ruby -w

class T_023no_x < Test
  def description
    return "mkvmerge / various --no-xyz options / in(*)"
  end

  def run
    merge("-A --no-attachments data/complex.mkv")
    hash = hash_tmp
    merge("-D --no-chapters data/complex.mkv")
    hash += "-" + hash_tmp
    merge("-S --no-tags data/complex.mkv")
    return hash + "-" + hash_tmp
  end
end

