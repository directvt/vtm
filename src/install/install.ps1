If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {   
$arguments = "& '" + $myinvocation.mycommand.definition + "'";
Start-Process powershell -Verb runAs -ArgumentList $arguments;
Break;
}

$PF = $ENV:ProgramFiles
$vtm_dir = "$PF\vtm"
$PrintBar = $("-" * 40)

$cwd = Split-Path -Path $myinvocation.mycommand.definition
cd $cwd
$PrintBar
"Current directory: $cwd"
$PrintBar
if (!($vtm_dir | Test-Path))
{
    "Create: $vtm_dir"
    mkdir $vtm_dir
    $PrintBar
}
"Copy: vtm*.exe to $vtm_dir"
$PrintBar
copy vtm*.exe $vtm_dir

$old_PATH_split = ($env:path).split(";")
if (-not $old_PATH_split.Contains($vtm_dir))
{
    $newpath = "$($env:path);$vtm_dir"
    "Update PATH=$env:path"
    $PrintBar
    $PATH_split = ($newpath).split(";")
    "New PATH="
    $PATH_split
    setx /M PATH "$($env:path);$vtm_dir"
}
else
{
    "Skip updating ENV:PATH: Path already exist ($vtm_dir)."
}

$PrintBar
cat ./readme.txt
$PrintBar
"
Restart command line environment in
order to be able to use vtm.
"
Write-Host -NoNewLine 'press any key...';
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown');
