

1) OMNeT 5.2.1 installieren ( auf plattformspezifische Vorbereitungen achten -> siehe Omnet Install Guide)
2) install and test .NET SDK for Linux -> https://www.microsoft.com/net/learn/get-started/linux/ubuntu16-04
2) Die .NET dlls sollten nun in /usr/share/dotnet/shared/Microsoft.NETCore.App/2.x.x liegen (
2) Repository colen $ git clone --recursive git@github.com:jensdrenhaus/inet.git 
3) auf Showcase-Branch wechseln $ checkout --track origin/dotnet_showcase
4) Ins INET wurzelverzechnis wechseln
3) $ make makefiles
3) $ make
4) inet/src/int/applications/extern/DotnetApplicationAdapter.ned öffnen und bei clrFilesPath den unter 3 erwänten Pfad einfügen/kontrollieren
5) zu inet/examples/phynettest_dotnet wechseln
5) Benutzerrechte von ./setup und ./ run kontrollieren. Diese müssen ausführbar sein.
5) C# Code bauen $ ./setup 
6) $ ./run
7) Im sich öffnenden GUI auf play klicken -> Node 1 sendet dauerfaft an Node 2

Die Funktionalität der App-Schicht kann in inet/src/scharp/OmnetApplication.cs implementiert werden.

