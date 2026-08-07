void voidMethod() {
  return
}

void voidMethod1() {
  # ERROR line=next message="Cannot return a value in a void method"
  return false
}

bool bMethod() {
  return true
}

string strMethod() {
  # ERROR line=next message="Returned value of type int8 cannot be assigned to expected type string"
  return 12
}

# ERROR line=next message="Non-void function has no return value"
string strMethod2() {

}