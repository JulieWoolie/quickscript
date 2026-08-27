const CACHE_PATH = `.cache`

export async function fetchCached(url: string, cacheFilename: string): Promise<string> {
  const cachePath = `${CACHE_PATH}/${cacheFilename}`
  let strContent = ""

  try {
    strContent = await Deno.readTextFile(cachePath)
    console.log(`Found cached data file ${cachePath}`)
  } catch (err) {
    if (!(err instanceof Deno.errors.NotFound)) {
      throw err
    }

    console.log(`Fetching requested data from ${url}`)

    const res = await fetch(url)
    if (!res.ok) {
      throw `Failed to fetch file ${url} (status: ${res.status}: ${res.statusText})`
    }

    try {
      await Deno.mkdir(`.cache/`)
    } catch (ignored) {}

    const file = await Deno.open(cachePath, {
      write: true,
      create: true,
      truncate: true
    })

    await res.body!!.pipeTo(file.writable)

    strContent = await Deno.readTextFile(cachePath)
  }

  return strContent
}