[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0, ValueFromRemainingArguments = $true)]
    [string[]] $QueryParts,

    [int]    $TopK   = 6,
    [string] $Server = 'http://doc-mcp.misc-server:3333/mcp',
    [switch] $Raw
)

$ErrorActionPreference = 'Stop'

$query = ($QueryParts -join ' ').Trim()
if (-not $query) { throw 'Empty query.' }

$payload = @{
    jsonrpc = '2.0'
    id      = [int][double]::Parse((Get-Date -UFormat %s))
    method  = 'tools/call'
    params  = @{
        name      = 'doc_search'
        arguments = @{ query = $query; top_k = $TopK }
    }
} | ConvertTo-Json -Depth 8 -Compress

$tmp = Join-Path $env:TEMP ("doc-search-{0}.json" -f ([guid]::NewGuid()))
Set-Content -Path $tmp -Value $payload -Encoding utf8 -NoNewline
try {
    $resp = & curl.exe -sS -m 30 -X POST $Server `
        -H 'Content-Type: application/json' `
        -H 'Accept: application/json, text/event-stream' `
        --data-binary "@$tmp"
} finally {
    Remove-Item $tmp -ErrorAction SilentlyContinue
}

if ($Raw) { $resp; return }

try { $obj = $resp | ConvertFrom-Json } catch { $resp; return }

if ($obj.error) {
    Write-Error ("MCP error {0}: {1}" -f $obj.error.code, $obj.error.message)
    return
}

$content = $obj.result.content
if (-not $content) { $resp; return }

foreach ($block in $content) {
    if ($block.type -eq 'text') {
        $block.text
        "`n----`n"
    } else {
        $block | ConvertTo-Json -Depth 6
    }
}
