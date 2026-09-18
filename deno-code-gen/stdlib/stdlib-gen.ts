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
  source?: string[]
}

interface Var extends ScriptSymbol {
  type: "constant"
  value?: string
}

type PrintSymbol = Var | Func

const SYMBOLS: PrintSymbol[] = []

function f(prio: number, inp: string, comments?: string[], source?: string[]): void {
  const retTypeEnd = inp.indexOf(' ')
  const stype = inp.substring(0, retTypeEnd)

  if (inp.includes("(") && !inp.includes("=")) {
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
      comments,
      source
    })

    return
  }

  let value: string | undefined = undefined
  let name: string = inp.substring(retTypeEnd + 1)

  if (inp.includes("=")) {
    const vStart = inp.indexOf('=')
    value = inp.substring(vStart + 1).trim()
    name = inp.substring(retTypeEnd + 1, vStart - 1)
  }

  SYMBOLS.push({
    type: "constant",
    comments,
    prio,
    stype,
    name,
    value
  })
}

function createStdSymbols(): void {
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

  // f(0, "void printf(string format, uint64... args)", [
  //   "Print a formatted message to the standard output",
  //   "",
  //   ...formatCodes
  // ])

  f(1, "void println(string message)", [
    "Print a message to the standard output"
  ])

  // f(2, "string sformat(string format, uint64... args)", [
  //   "Format a string",
  //   "",
  //   ...formatCodes
  // ])

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

  f(3.1, "float64 NaN = -(0.0 / 0.0)", ["Not-A-Number"])
  f(3.2, "float64 POSITIVE_INFINITY = 1.0 / 0.0", ["Positive infinity"])
  f(3.3, "float64 NEGATIVE_INFINITY = -(1.0 / 0.0)", ["Negative infinity"])

  f(3.4, `float64 PI = ${Math.PI}`, ["Ratio of the circumference of a circle to its diameter"])
  f(3.5, `float64 E = ${Math.E}`, ["Euler's number", "Base of the natural logarithm"])
  f(3.6, `float64 TAU = ${Math.PI * 2}`, ["Ratio of the circumference of a circle to its radius"])
  f(3.6, `float64 LN10 = ${Math.LN10}`, ["Natural logarithm of 10"])
  f(3.7, `float64 LN2 = ${Math.LN2}`, ["Natural logarithm of 2"])
  f(3.8, `float64 LOG10E = ${Math.LOG10E}`, ["Base-10 logarithm of E"])
  f(3.9, `float64 LOG2E = ${Math.LOG2E}`, ["Base-2 logarithm of E"])
  f(3.91, `float64 SQRT1_2 = ${Math.SQRT1_2}`, ["Square root of one divided by two"])
  f(3.92, `float64 SQRT2 = ${Math.SQRT2}`, ["Square root of two"])

  for (const nt of NUMBER_TYPES) {
    const fn = nt.fullname;

    if (nt.signed) {
      f(4, `${fn} abs(${fn} x)`, [
        "Get the absolute value of a number",
        "",
        "If the argument is non-negative, the argument is returned",
        "Otherwise the argument is negated and returned.",
      ], [
        "return x < 0 ? -x : x"
      ])

      f(7, `int8 sign(${fn} x)`, [
        "Get the sign of a number",
        "If the argument is 0, 0 is returned",
        "If the argument is negative, -1 is returned",
        "If the argument is positive, 1 is returned"
      ], [
        "return x < 0 ? -1 : (x > 0 ? 1 : 0)"
      ])
    }

    f(5, `${fn} min(${fn}... values)`, [
      "Get the smallest value"
    ], [
      "if values.length == 0 return 0",
      ``,
      `${fn} r = values[0]`,
      `${fn} v = 0`,
      `const uint32 len = values.length`,
      ``,
      `for (uint32 i = 1; i < len; ++i) {`,
      `  r = (v = values[i]) < r ? v : r`,
      `}`,
      ``,
      `return r`
    ])

    f(6, `${fn} max(${fn}... values)`, [
      "Get the largest value"
    ], [
      "if values.length == 0 return 0",
      ``,
      `${fn} r = values[0]`,
      `${fn} v = 0`,
      `const uint32 len = values.length`,
      ``,
      `for (uint32 i = 1; i < len; ++i) {`,
      `  r = (v = values[i]) > r ? v : r`,
      `}`,
      ``,
      `return r`
    ])

    if (nt.integral) {
      f(8, `${fn} clz(${fn} x)`, [
        "Get the number of leading zeros"
      ])
      continue
    }

    f(8.5, `bool isNaN(${fn} x)`, ["Test if a value is Not-A-Number"], [`return x != x`])
    f(8.6, `bool isInf(${fn} x)`, ["Test if a value is infinite"])
    f(8.7, `bool isFinite(${fn} x)`, ["Test if a value is finite"])
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
}

function sortSymbols() {
  SYMBOLS.forEach(s => {
    if (s.type == "constant") {
      s.prio = s.prio - 100
    }
  })

  SYMBOLS.sort((a, b) => a.prio - b.prio)
}

async function generateStdLibDeclFile(): Promise<void> {
  let out = `#
# quickscript Standard Library
# Version 0
#

native module std from "stdlib"`
  for (const func of SYMBOLS) {
    out += `\n`

    if (func.comments) {
      out += `\n/**`
      for (const cl of func.comments) {
        out += `\n * ${cl}`
      }
      out += `\n */`
    }

    out += `\nexport `

    if (func.type == "constant" || (func.source != undefined && func.source.length < 2)) {
      out += `const `
    } else if (func.source == undefined) {
      out += "native "
    }

    out += `${func.stype} ${func.name}`

    if (func.type == "constant") {
      if (func.value != undefined) {
        out += ` = ${func.value}`
      }
      continue
    }

    if (func.type == "func") {
      const params = func.params.map(p => `${p.tn} ${p.pname}`).join(", ")
      out += `(${params})`

      if (func.source != undefined) {
        out += ` {`
        for (const s of func.source) {
          out += `\n  ${s}`
        }
        out += `\n}`
      }
    }
  }

  await writeToFile(out, "../stdlib.qscr")
}

function getNativeFunctionName(sym: Func): string {
  let str = `qs_stdlib_${sym.name}`
  sym.params.forEach((v) => {
    let tn = v.tn
    if (tn.endsWith("...")) {
      tn = `${tn.substring(0, tn.length - 3)}va`
    }
    str += `_${tn}`
  })
  return str
}

function signatureToString(f: Func): string {
  let pStr = f.params.map(v => v.tn).join(",")

  if (f.stype == "void") {
    return `(${pStr})`
  }

  return `(${pStr})=>${f.stype}`
}

async function generateStdLibHeader(): Promise<void> {
  let out = `#ifndef QS_STDLIB
#define QS_STDLIB

#include "qsni.h"

QS_EXPORT void QS_CALL qs_onLoadNativeModule(QsEnv env, conststring ns);

#endif // QS_STDLIB`
  await writeToFile(out, "../libraries/stdlib/stdlib.hpp")
}

async function generateStdLibSource(): Promise<void> {
  let out = `#include "stdlib.hpp"`
  const funcs = SYMBOLS.filter(s => s.type == "func" && s.source == undefined) as Func[]

  for (const sym of funcs) {
    out += `\n\n// export native ${sym.stype} ${sym.name}(${sym.params.map(t => `${t.tn} ${t.pname}`).join(', ')})`
    out += `\nstatic void ${getNativeFunctionName(sym)}(const QsVirtualMachine vm, const QsNativeCall call) {`

    let pIdx = 0
    for (const p of sym.params) {
      let argShorthand: string = ""
      let tn = p.tn

      if (tn == "string" || tn.endsWith("[]") || tn.endsWith("...")) {
        tn = "QsScriptArray"
        argShorthand = "Array"
      } else if (tn.startsWith("uint")) {
        argShorthand = `U${tn.substring(4)}`
      } else if (tn.startsWith("int")) {
        argShorthand = `I${tn.substring(3)}`
      } else if (tn.startsWith("float")) {
        argShorthand = `F${tn.substring(tn.length - 2)}`
      } else if (tn == "bool") {
        tn = "boolean"
        argShorthand = "Bool"
      } else {
        argShorthand = `${tn.substring(0, 1).toUpperCase()}${tn.substring(1)}`
      }

      out += `\n  const ${tn} ${p.pname} = qsc_get${argShorthand}Argument(call, ${pIdx++});`
    }

    out += `\n  // Empty generated function stub\n}`
  }

  out += `\n\nvoid qs_onLoadNativeModule(QsEnv env, conststring ns) {
  qse_registerNatives(env, ns, ${funcs.length}`

  const args: string[][] = []
  const biggest: number[] = [0, 0, 0]

  for (const sym of funcs) {
    const signStr = signatureToString(sym)
    const funcName = getNativeFunctionName(sym)

    let arr = [`"${sym.name}",`, `"${signStr}",`, funcName]
    arr.forEach((s, i) => {
      biggest[i] = Math.max(s.length, biggest[i])
    })

    args.push(arr)
  }

  args.forEach(v => {
    out += `,\n    `
    v.forEach((str, i) => {
      if (i != (v.length - 1)) {
        out += str.padEnd(biggest[i], ' ') + " "
      } else {
        out += str
      }
    })
  })

  out += "\n  );\n}"

  await writeToFile(out, "../libraries/stdlib/stdlib.cpp")
}

export async function generateStdLib(): Promise<void> {
  createStdSymbols()
  sortSymbols()

  await generateStdLibDeclFile()
  await generateStdLibHeader()
  await generateStdLibSource()
}