void main() {
  int32[] arr = [1, 2]

  uint32 len = arr.length

  int32 p = arr[0]

  # ERROR line=next message="Cannot mutate array length"
  arr.length = 4

  # ERROR line=next message="No such property 'something' on int32[]"
  arr.something

  arr[0] = 4

  int32[][] xy = [[0, 0], [1, 2], [3, 4]]
  int32[] something = xy[0]

  # ERROR line=next message="Cannot use value of type string in array of type int32"
  int32[] nonFitting = ["foo"]
}