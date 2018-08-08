using System;
using System.Threading.Tasks;
using Akka.Actor;
using Akka.Actor.Internal;
using Akka.Event;
using Akka.Routing;
using PhyNetFlow.Core.Akka;

namespace PhyNetFlow.OMNeT
{
    public class OmnetActorRefProvider: IActorRefProvider
    {
        private readonly IActorRefProvider _localActorRefProvider;

        public ISimulationMessageDelegate SimulationMessageDelegate { get; set; }

        public OmnetActorRefProvider(string systemName, Settings settings, EventStream eventStream)
        {
            _localActorRefProvider = new LocalActorRefProvider(systemName, settings, eventStream);
        }

        public IActorRef RootGuardianAt(Address address)
        {
            return _localActorRefProvider.RootGuardianAt(address);
        }

        public void Init(ActorSystemImpl system)
        {
            _localActorRefProvider.Init(system);
        }

        public ActorPath TempPath()
        {
            return _localActorRefProvider.TempPath();
        }

        public void RegisterTempActor(IInternalActorRef actorRef, ActorPath path)
        {
            _localActorRefProvider.RegisterTempActor(actorRef, path);
        }

        public void UnregisterTempActor(ActorPath path)
        {
            _localActorRefProvider.UnregisterTempActor(path);
        }

        /// <summary>
        /// Initialized SimulationActorRefs when necessary and LocalActorRefs otherwise.
        /// </summary>
        /// <param name="system"></param>
        /// <param name="props"></param>
        /// <param name="supervisor"></param>
        /// <param name="path"></param>
        /// <param name="systemService"></param>
        /// <param name="deploy"></param>
        /// <param name="lookupDeploy"></param>
        /// <param name="async"></param>
        /// <returns></returns>
        public IInternalActorRef ActorOf(ActorSystemImpl system, Props props, IInternalActorRef supervisor, ActorPath path,
            bool systemService, Deploy deploy, bool lookupDeploy, bool async)
        {
            // TODO: This should definitely be moved somewhere else.
            if (!system.Dispatchers.HasDispatcher(CallingThreadDispatcher.Id)) {
                system.Dispatchers.RegisterConfigurator(
                    CallingThreadDispatcher.Id,
                    new CallingThreadDispatcherConfigurator(system.Settings.Config, system.Dispatchers.Prerequisites)
                );
            }

            // If the instance that needs to be created is
            // - a SystemService
            // - or a Router
            // just return a regular LocalActorRef, because
            // we currently do not render messages sent to the router or system messages.
            if (systemService || !(props.Deploy.RouterConfig is NoRouter))
            {
                return _localActorRefProvider.ActorOf(system, props.WithDispatcher(CallingThreadDispatcher.Id), supervisor, path, systemService, deploy, lookupDeploy, async);
            }

            // Create dispatchers and mailboxes according to the configurations.
            var dispatcher = system.Dispatchers.Lookup(CallingThreadDispatcher.Id);
            var mailboxType = system.Mailboxes.GetMailboxType(props, dispatcher.Configurator.Config);
            return new SimulationActorRef(SimulationMessageDelegate, system, props, dispatcher, mailboxType, supervisor, path);
        }

        public IActorRef ResolveActorRef(string path)
        {
            return _localActorRefProvider.ResolveActorRef(path);
        }

        public IActorRef ResolveActorRef(ActorPath actorPath)
        {
            return _localActorRefProvider.ResolveActorRef(actorPath);
        }

        public Address GetExternalAddressFor(Address address)
        {
            return _localActorRefProvider.GetExternalAddressFor(address);
        }

        public IInternalActorRef RootGuardian => _localActorRefProvider.RootGuardian;

        public LocalActorRef Guardian => _localActorRefProvider.Guardian;

        public LocalActorRef SystemGuardian => _localActorRefProvider.SystemGuardian;

        public IActorRef DeadLetters => _localActorRefProvider.DeadLetters;

        public ActorPath RootPath => _localActorRefProvider.RootPath;

        public Settings Settings => _localActorRefProvider.Settings;

        public Deployer Deployer => _localActorRefProvider.Deployer;

        public IInternalActorRef TempContainer => _localActorRefProvider.TempContainer;

        public Task TerminationTask => _localActorRefProvider.TerminationTask;

        public Address DefaultAddress => _localActorRefProvider.DefaultAddress;
    }
}
