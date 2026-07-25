int32 main() {
  string str = "Hello, world!"
  char ch = str[0]

  # ERROR line=next message="Cannot mutate strings"
  str[0] = 'h'
}