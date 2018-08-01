Momentan nur für Linux (Ubuntu 16.04 LTS getestet)

1) OMNeT 5.2.1 installieren ( auf plattformspezifische Vorbereitungen und Nachbereitungen achten -> siehe Omnet Install Guide)
2) .NET SDK for Linux istallieren und testen -> https://www.microsoft.com/net/learn/get-started/linux/ubuntu16-04
3) Die .NET Dlls sollten nun in /usr/share/dotnet/shared/Microsoft.NETCore.App/2.x.x liegen (
4) Repository colen $ git clone --recursive git@github.com:jensdrenhaus/inet.git 
5) auf Showcase-Branch wechseln $ checkout --track origin/dotnet_showcase_akka
6) ins INET Wurzelverzechnis wechseln
7) $ make makefiles
8) $ make
9) inet/src/int/applications/extern/DotnetApplicationAdapter.ned öffnen und bei clrFilesPath den unter 3 erwänten Pfad einfügen/kontrollieren
10) zu inet/examples/phynettest_dotnet wechseln
11) Benutzerrechte von ./setup und ./ run kontrollieren. Diese müssen ausführbar sein.
12) C# Code bauen $ ./setup 
13) $ ./run
14) Im sich öffnenden GUI auf play klicken -> PingPong Szenario

Die Funktionalitet der App-Schicht kann in inet/src/scharp/OmnetApplication.cs implementiert werden.

Bekannte Fehlerquellen: 
in ~/.nuget/packages darf sich keine ältere/ andere version von akka befinden (nur akka 1.3.8)
