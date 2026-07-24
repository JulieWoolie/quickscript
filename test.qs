struct vec2f {
  float32 x = 0.0
  float32 y = 0.0
}

int32 main(string[] args) {
  vec2f v = {}

  bool b = (v.x + v.y) == 0.0
  int32 i = b ? 1 : 0

  i * 2.0

  print(args[0])

  return 0
}