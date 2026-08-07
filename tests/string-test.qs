void main() {
  string str = "Hello, world!"
  char ch = str[0]

  # ERROR line=next message="Cannot mutate strings"
  str[0] = 'h'

  uint32 len = str.length

  # ERROR line=next message="Cannot mutate string length"
  str.length = 23
}