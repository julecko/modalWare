$cert = New-SelfSignedCertificate -Type CodeSigning -Subject "CN=Dilino s.r.o." -KeySpec Signature -CertStoreLocation Cert:\CurrentUser\My

$certPath = "$PWD\Certificate.pfx"
$password = ConvertTo-SecureString -String "Password1234" -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath $certPath -Password $password

Get-ChildItem -Path $PWD
signtool sign /f ".\Certificate.pfx" /p "Password1234" /t "http://timestamp.sectigo.com" /fd SHA256 /v "..\x64\Release\modalWare.exe"