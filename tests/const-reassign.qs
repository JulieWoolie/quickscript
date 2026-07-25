int32 main() {
  const uint32 x = 0

  # ERROR line=next message="Cannot reassign const variable 'x'"
  x = 1

  # ERROR line=next message="Cannot reassign const variable 'x'"
  x++
}