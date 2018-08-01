Momentan nur für Linux (Ubuntu 16.04 LTS getestet)

1) OMNeT 5.2.1 installieren ( auf plattformspezifische Vorbereitungen und Nachbereitungen achten -> siehe Omnet Install Guide)
2) .NET SDK für Linux istallieren und testen -> https://www.microsoft.com/net/learn/get-started/linux/ubuntu16-04
3) Die .NET Dlls sollten nun in /usr/share/dotnet/shared/Microsoft.NETCore.App/2.x.x liegen 
4) Repository colen $ git clone --recursive git@github.com:jensdrenhaus/inet.git 
5) auf Dotnet-Branch wechseln $ checkout --track origin/dotnetcore
6) ins INET Wurzelverzechnis wechseln
7) $ make makefiles
8) $ make

Beispiel starten: z.B "akka pingpong"

1) inet/examples/phynet_dotnet_akka_pingpong/PhyNetDynamic.ned öffnen und bei Parameter 'clrFilesPath' den unter 3 genannten Pfad einfügen/kontrollieren (Versionsnummer)
2) zu inet/examples/phynettest_dotnet_akka_pingpong wechseln
3) Benutzerrechte von ./setup und ./ run kontrollieren. Diese müssen ausführbar sein.
4) C# Code bauen $ ./setup 
5) Simulation ausführen $ ./run
6) Im sich öffnenden GUI oben auf "play" klicken -> PingPong Szenario

Die Funktionalität der App-Schicht kann in inet/src/scharp/OmnetApplication.cs implementiert werden.

Bekannte Fehlerquellen: 
in ~/.nuget/packages darf sich keine ältere/andere version von akka befinden (nur akka 1.3.8)
