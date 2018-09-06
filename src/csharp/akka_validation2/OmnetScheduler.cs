using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Threading;
using Akka.Actor;
using Akka.Configuration;
using Akka.Event;
using OmnetServices;

namespace PhyNetFlow.OMNeT
{
    // https://msdn.microsoft.com/en-us/library/367eeye0.aspx
    // ReSharper disable once UnusedMember.Global
    public class OmnetScheduler : IScheduler, IAdvancedScheduler
    {
        private readonly ConcurrentDictionary<long,ConcurrentQueue<ScheduledItem>> _scheduledWork;
        public TimeSpan MonotonicClock => TimeSpan.FromMilliseconds(OmnetSimulation.Instance().GetGlobalTime() - OmnetInterface.Instance.TimeAtStart);
        public TimeSpan HighResMonotonicClock => TimeSpan.FromMilliseconds(OmnetSimulation.Instance().GetGlobalTime() - OmnetInterface.Instance.TimeAtStart);

        protected DateTimeOffset TimeNow =>
            OmnetInterface.Instance.DateTimeOffsetAtStart.AddMilliseconds(OmnetSimulation.Instance().GetGlobalTime());

        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="schedulerConfig">TBD</param>
        /// <param name="log">TBD</param>
        public OmnetScheduler(Config schedulerConfig, ILoggingAdapter log)
        {
            _scheduledWork = new ConcurrentDictionary<long, ConcurrentQueue<ScheduledItem>>();
            OmnetInterface.Instance.OmnetGlobalTimerNotify += InstanceOnOmneTTimerNotify;
        }

        private void InstanceOnOmneTTimerNotify()
        {
            // Once the timer is triggered, trigger all work that has been scheduled in the meantime.
            Advance();
        }

        /// <summary>
        /// TBD
        /// </summary>
        public void Advance()
        {
            var tickItems = _scheduledWork
                .Where(s => s.Key <= Now.Ticks)
                .OrderBy(s => s.Key)
                .ToList();
            foreach (var t in tickItems)
            {
                foreach (var si in t.Value.Where(i => i.Cancelable == null || !i.Cancelable.IsCancellationRequested))
                {
                    if (si.Type == ScheduledItem.ScheduledItemType.Message)
                        si.Receiver.Tell(si.Message, si.Sender);
                    else
                        si.Action();
                    si.DeliveryCount++;
                }
                _scheduledWork.TryRemove(t.Key, out var removed);
                foreach (var i in removed.Where(r => r != null && r.Repeating && (r.Cancelable == null || !r.Cancelable.IsCancellationRequested)))
                {
                    InternalSchedule(null, i.Delay, i.Receiver, i.Message, i.Action, i.Sender, i.Cancelable, i.DeliveryCount);
                }
            }
        }

        private void InternalSchedule(TimeSpan? initialDelay, TimeSpan delay, ICanTell receiver, object message, Action action,
            IActorRef sender, ICancelable cancelable, int deliveryCount = 0)
        {
            var scheduledTime = TimeNow.Add(initialDelay ?? delay).UtcTicks;
            if (!_scheduledWork.TryGetValue(scheduledTime, out var tickItems))
            {
                tickItems = new ConcurrentQueue<ScheduledItem>();
                _scheduledWork.TryAdd(scheduledTime, tickItems);
            }
            var type = message == null ? ScheduledItem.ScheduledItemType.Action : ScheduledItem.ScheduledItemType.Message;
            tickItems.Enqueue(new ScheduledItem(initialDelay ?? delay, delay, type, message, action,
                initialDelay.HasValue || deliveryCount > 0, receiver, sender, cancelable));
            
            // TODO: This probably needs adjustment for repetative events?
            OmnetSimulation.Instance().SetGlobalTimerMillisecounds((int)(initialDelay ?? delay).TotalMilliseconds);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="delay">TBD</param>
        /// <param name="receiver">TBD</param>
        /// <param name="message">TBD</param>
        /// <param name="sender">TBD</param>
        public void ScheduleTellOnce(TimeSpan delay, ICanTell receiver, object message, IActorRef sender)
        {
            InternalSchedule(null, delay, receiver, message, null, sender, null);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="delay">TBD</param>
        /// <param name="receiver">TBD</param>
        /// <param name="message">TBD</param>
        /// <param name="sender">TBD</param>
        /// <param name="cancelable">TBD</param>
        public void ScheduleTellOnce(TimeSpan delay, ICanTell receiver, object message, IActorRef sender, ICancelable cancelable)
        {
            InternalSchedule(null, delay, receiver, message, null, sender, cancelable);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="initialDelay">TBD</param>
        /// <param name="interval">TBD</param>
        /// <param name="receiver">TBD</param>
        /// <param name="message">TBD</param>
        /// <param name="sender">TBD</param>
        public void ScheduleTellRepeatedly(TimeSpan initialDelay, TimeSpan interval, ICanTell receiver, object message,
            IActorRef sender)
        {
            InternalSchedule(initialDelay, interval, receiver, message, null, sender, null);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="initialDelay">TBD</param>
        /// <param name="interval">TBD</param>
        /// <param name="receiver">TBD</param>
        /// <param name="message">TBD</param>
        /// <param name="sender">TBD</param>
        /// <param name="cancelable">TBD</param>
        public void ScheduleTellRepeatedly(TimeSpan initialDelay, TimeSpan interval, ICanTell receiver, object message,
            IActorRef sender, ICancelable cancelable)
        {
            InternalSchedule(initialDelay, interval, receiver, message, null, sender, cancelable);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="delay">TBD</param>
        /// <param name="action">TBD</param>
        /// <param name="cancelable">TBD</param>
        public void ScheduleOnce(TimeSpan delay, Action action, ICancelable cancelable)
        {
            InternalSchedule(null, delay, null, null, action, null, null);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="delay">TBD</param>
        /// <param name="action">TBD</param>
        public void ScheduleOnce(TimeSpan delay, Action action)
        {
            InternalSchedule(null, delay, null, null, action, null, null);
        }
        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="initialDelay">TBD</param>
        /// <param name="interval">TBD</param>
        /// <param name="action">TBD</param>
        /// <param name="cancelable">TBD</param>
        public void ScheduleRepeatedly(TimeSpan initialDelay, TimeSpan interval, Action action, ICancelable cancelable)
        {
            InternalSchedule(initialDelay, interval, null, null, action, null, cancelable);
        }

        /// <summary>
        /// TBD
        /// </summary>
        /// <param name="initialDelay">TBD</param>
        /// <param name="interval">TBD</param>
        /// <param name="action">TBD</param>
        public void ScheduleRepeatedly(TimeSpan initialDelay, TimeSpan interval, Action action)
        {
            InternalSchedule(initialDelay, interval, null, null, action, null, null);
        }
        /// <summary>
        /// TBD
        /// </summary>
        public DateTimeOffset Now { get { return TimeNow; } }
        /// <summary>
        /// TBD
        /// </summary>
        public IAdvancedScheduler Advanced
        {
            get { return this; }
        }
        /// <summary>
        /// TBD
        /// </summary>
        internal class ScheduledItem
        {
            /// <summary>
            /// TBD
            /// </summary>
            public TimeSpan InitialDelay { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public TimeSpan Delay { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public ScheduledItemType Type { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public object Message { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public Action Action { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public bool Repeating { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public ICanTell Receiver { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public IActorRef Sender { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public ICancelable Cancelable { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public int DeliveryCount { get; set; }
            /// <summary>
            /// TBD
            /// </summary>
            public enum ScheduledItemType
            {
                /// <summary>
                /// TBD
                /// </summary>
                Message,
                /// <summary>
                /// TBD
                /// </summary>
                Action
            }
            /// <summary>
            /// TBD
            /// </summary>
            /// <param name="initialDelay">TBD</param>
            /// <param name="delay">TBD</param>
            /// <param name="type">TBD</param>
            /// <param name="message">TBD</param>
            /// <param name="action">TBD</param>
            /// <param name="repeating">TBD</param>
            /// <param name="receiver">TBD</param>
            /// <param name="sender">TBD</param>
            /// <param name="cancelable">TBD</param>
            public ScheduledItem(TimeSpan initialDelay, TimeSpan delay, ScheduledItemType type, object message, Action action, bool repeating, ICanTell receiver,
                IActorRef sender, ICancelable cancelable)
            {
                InitialDelay = initialDelay;
                Delay = delay;
                Type = type;
                Message = message;
                Action = action;
                Repeating = repeating;
                Receiver = receiver;
                Sender = sender;
                Cancelable = cancelable;
                DeliveryCount = 0;
            }
        }
    }
}
