int32 main() {
  int32[] arr = [1, 2]

  uint32 len = arr.length

  int32 p = arr[0]

  # ERROR line=next message="Cannot mutate array length"
  arr.length = 4

  # ERROR line=next message="No such property 'something' on int32[]"
  arr.something
}