# I dislike PowerShell but this is pretty much the only way to do this that I know

$outf = "../keyw_lookup.cc"

gperf -t qs-keywords.gperf > $outf

$cnt = (Get-Content $outf) -join "`n"

$cnt = $cnt.Replace('const struct keyword *', 'tokentype')
$cnt = $cnt.Replace('return 0;', 'return TT_UNKNOWN;')
$cnt = $cnt.Replace('const char *s = wordlist[key].name;', 'keyword k = wordlist[key]; const char *s = k.name;')
$cnt = $cnt.Replace('return &wordlist[key];', 'return k.token;')
$cnt = $cnt.Replace('register ', '')
$cnt = $cnt.Replace("#ifdef __GNUC__`n__inline`n#endif", '')
$cnt = $cnt.Replace("tokentype`n", 'tokentype ')
$cnt = $cnt.Replace("const char *", 'conststring ')
$cnt = $cnt.Replace("unsigned int", 'uint32')

$cnt | Set-Content $outf
