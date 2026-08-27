import {NUMBER_TYPES, writeToFile} from "../common";

interface FuncParam {
  tn: string,
  pname: string
}

interface ScriptSymbol {
  type: "func" | "constant"
  prio: number
  comments?: string[] | undefined
  name: string
  stype: string
}

interface Func extends ScriptSymbol {
  type: "func"
  params: FuncParam[]
}

interface Var extends ScriptSymbol {
  type: "constant"
}

type PrintSymbol = Var | Func

const SYMBOLS: PrintSymbol[] = []

function f(prio: number, inp: string, comments?: string[]): void {
  const retTypeEnd = inp.indexOf(' ')
  const stype = inp.substring(0, retTypeEnd)

  if (inp.includes("(")) {
    const nameEnd = inp.indexOf('(')
    const fname = inp.substring(retTypeEnd + 1, nameEnd)

    const paramsEnd = inp.indexOf(')')

    const paramStrings = inp.substring(nameEnd + 1, paramsEnd).trim()
    let params: FuncParam[]

    if (paramStrings.length == 0) {
      params = []
    } else {
      params = paramStrings
          .split(",")
          .map((v) => v.trim())
          .map((v) => v.split(" "))
          .map((pair) => {return {tn: pair[0], pname: pair[1]}})
    }

    SYMBOLS.push({
      type: "func",
      name: fname,
      params,
      prio,
      stype,
      comments
    })

    return
  }

  SYMBOLS.push({
    type: "constant",
    comments,
    prio,
    stype,
    name: inp.substring(retTypeEnd + 1)
  })
}

export async function generateStdLibDeclFile(): Promise<void> {
  const formatCodes: string[] = [
    "FORMAT CODES:",
    "- '%d' Decimal number",
    "- '%x' Hexadecimal number",
    "- '%X' Uppercase hexadecimal number",
    "- '%o' Base 8 (octal) number",
    "- '%O' Upper case base 8 (octal) number",
    "- '%b' Binary base 2 number",
    "- '%s' Convert the argument to string",
    "- '%c' Convert the argument to a UTF-32 unicode character",
    "- '%e' Convert the argument to scientific notation",
    "- '%t' Treat the argument as a UNIX timestamp and print the date time",
    "",
    "FLAGS:",
    "- <number>  Pad the string with spaces to the left or right",
    "            (depending on if the number is negative or not)",
    "            Example: '%3s'",
    "- 0<number> Pad the string with '0' characters to the left or right",
    "            (depending on if the number is negative or not)",
    "            Example: '%03s'",
    "- .<number> Number precision flag",
    "            Example: '%.2d'",
    "- $<number> Argument index",
    "            Example: '%$2s'",
    "",
    "EXAMPLES:",
    `- printf("Hello world!\\n") Prints hello world with a newline character at the end`,
    `- printf("number: %d", 23) Prints the number`,
    ""
  ]

  f(0, "void printf(string format, ... args)", [
    "Print a formatted message to the standard output",
    "",
    ...formatCodes
  ])

  f(1, "void println(string message)", [
    "Print a message to the standard output"
  ])

  f(2, "string sformat(string format, ... args)", [
    "Format a string",
    "",
    ...formatCodes
  ])

  f(3, "uint64 currentTimeMillis()", [
    "Get the current time as a UNIX timestamp"
  ])

  f(3.01, "string getenv(string name)", [
      "Get the value of an environment variable",
      "",
      "If the variable was not found, an empty string is returned"
  ])

  f(3.01, "bool setenv(string name, string value)", [
      "Set the value of an environment variable",
      "",
      "If the variable is already defined this function will return false",
      "",
      "The environment variable will only be changed for the current process and any processes",
      "spawned by the current process"
  ])
  f(3.02, "bool setenv(string name, string value, bool overwrite)", [
      "Set the value of an environment variable",
      "",
      "If the variable is defined and overwrite is set to true, then the value will be overridden",
      "and this function will return true, otherwise, if overwrite is set to false, then this will",
      "return false",
      "",
      "The environment variable will only be changed for the current process and any processes",
      "spawned by the current process"
  ])

  f(3.1, "float64 NaN", ["Not-A-Number"])
  f(3.2, "float64 POSITIVE_INFINITY", ["Positive infinity"])
  f(3.3, "float64 NEGATIVE_INFINITY", ["Negative infinity"])

  f(3.4, "float64 PI", ["Ratio of the circumference of a circle to its diameter"])
  f(3.5, "float64 E", ["Euler's number", "Base of the natural logarithm"])
  f(3.6, "float64 TAU", ["Ratio of the circumference of a circle to its radius"])
  f(3.6, "float64 LN10", ["Natural logarithm of 10"])
  f(3.7, "float64 LN2", ["Natural logarithm of 2"])
  f(3.8, "float64 LOG10E", ["Base-10 logarithm of E"])
  f(3.9, "float64 LOG2E", ["Base-2 logarithm of E"])
  f(3.11, "float64 SQRT1_2", ["Square root of one divided by two"])
  f(3.12, "float64 SQRT2", ["Square root of two"])

  for (const nt of NUMBER_TYPES) {
    const fn = nt.fullname;

    if (nt.signed) {
      f(4, `${fn} abs(${fn} x)`, [
        "Get the absolute value of a number",
        "",
        "If the argument is non-negative, the argument is returned",
        "Otherwise the argument is negated and returned.",
      ])

      f(7, `int8 sign(${fn} x)`, [
        "Get the sign of a number",
        "If the argument is 0, 0 is returned",
        "If the argument is negative, -1 is returned",
        "If the argument is positive, 1 is returned"
      ])
    }

    f(5, `${fn} min(${fn}... values)`, [
      "Get the smallest value"
    ])

    f(6, `${fn} max(${fn}... values)`, [
      "Get the largest value"
    ])

    if (nt.integral) {
      f(8, `${fn} clz(${fn} x)`, [
        "Get the number of leading zeros"
      ])
      continue
    }

    f(8.5, `bool isNaN(${fn} x)`, ["Test if a value is Not-A-Number"])
    f(9, `${fn} sqrt(${fn} x)`, ["Square root function"])
    f(10, `${fn} cbrt(${fn} x)`, ["Cube root function"])
    f(11, `${fn} acos(${fn} x)`, ["Get the arc cosine of a value"])
    f(12, `${fn} acosh(${fn} x)`, ["Get the inverse hyperbolic cosine of a value"])
    f(13, `${fn} asin(${fn} x)`, ["Get the arc sine of a value"])
    f(14, `${fn} asinh(${fn} x)`, ["Get the inverse hyperbolic sine of a value"])
    f(15, `${fn} atan(${fn} x)`, ["Get the arc tangent of a value"])
    f(16, `${fn} atan2(${fn} x, ${fn} y)`, ["Get the arctangent of the quotient of x and y"])
    f(17, `${fn} atanh(${fn} x)`, ["Get the hyperbolic arc tangent of a value"])
    f(18, `${fn} ceil(${fn} x)`, ["Round up"])
    f(18.1, `${fn} floor(${fn} x)`, ["Round down"])
    f(19, `${fn} cos(${fn} x)`, ["Get the cosine of a value"])
    f(20, `${fn} cosh(${fn} x)`, ["Get the inverse hyperbolic cosine of a value"])
    f(21, `${fn} hypot(${fn}... values)`, ["Get the square root of the sum of the squares of the arguments"])
    f(22, `${fn} log(${fn} x)`, ["Get the natural logarithm of a value"])
    f(23, `${fn} log10(${fn} x)`, ["Get the Base-10 logarithm of a value"])
    f(23, `${fn} log1p(${fn} x)`, ["Get the natural logarithm of a value + 1.0"])
    f(25, `${fn} log2(${fn} x)`, ["Get the Base-2 logarithm of a value"])
    f(26, `${fn} round(${fn} x)`, ["Round a floating point value"])
    f(27, `${fn} round(${fn} x, uint32 precision)`, ["Round a floating point value to a specific precision"])
    f(28, `${fn} sin(${fn} x)`, ["Get the sine of a value"])
    f(29, `${fn} sinh(${fn} x)`, ["Get the hyperbolic sine of a value"])
    f(30, `${fn} tan(${fn} x)`, ["Get the tangent of a value"])
  }

  SYMBOLS.forEach(s => {
    if (s.type == "constant") {
      s.prio = s.prio - 100
    }
  })

  SYMBOLS.sort((a, b) => a.prio - b.prio)

  let out = `#
# quickscript Standard Library
# Version 0
#`
  for (const func of SYMBOLS) {
    out += `\n`

    if (func.comments) {
      out += `\n/**`
      for (const cl of func.comments) {
        out += `\n * ${cl}`
      }
      out += `\n */`
    }

    out += `\nexport native `

    if (func.type == "constant") {
      out += `const `
    }

    out += `${func.stype} ${func.name}`

    if (func.type == "func") {
      const params = func.params.map(p => `${p.tn} ${p.pname}`).join(", ")
      out += `(${params})`
    }
  }

  await writeToFile(out, "../stdlib.qscr")
}