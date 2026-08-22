function fibonacci(n: number) {
  if (n == 0) return 0
  if (n == 1) return 1

  let gparent = 0
  let parent = 1

  for (let i = 0; i < n - 1; i++) {
    const r = gparent + parent
    gparent = parent
    parent = r
  }

  return parent
}

console.log(fibonacci(15))