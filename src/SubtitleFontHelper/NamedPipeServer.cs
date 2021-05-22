using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Pipes;
using System.IO;
using System.Threading;
using Google.Protobuf;

namespace SubtitleFontHelper
{
    class NamedPipeClientInfo
    {
        public int ProcessId { get; set; }
        public NamedPipeServerStream Stream { get; set; }
        public Thread Thread { get; set; }
        public MessageProcessor Processor { get; set; }
    }

    class NamedPipeServer
    {
        private List<NamedPipeClientInfo> _info = new List<NamedPipeClientInfo>();
        private readonly CancellationTokenSource _cancellation = new CancellationTokenSource();
        private NamedPipeServerStream _listening;
        private volatile bool _running = false;

        private readonly object _stop_lock = new object();

        public string NamedPipePath { get; private set; }
        public int MaxInstances { get; private set; }
        public CancellationTokenSource Cancellation => _cancellation;
        public bool Running => _running;

        public NamedPipeServer(string pipePath, int maxInstance = NamedPipeServerStream.MaxAllowedServerInstances)
        {
            NamedPipePath = pipePath;
            MaxInstances = maxInstance;

            Cancellation.Token.Register(delegate ()
            {
                lock (_info)
                {
                    foreach(var info in _info)
                    {
                        info.Stream.Dispose();
                    }
                    _info.Clear();
                }
                lock (_stop_lock)
                {
                    _listening.Dispose();
                }
            });
        }

        private void OnNewConnection(NamedPipeServerStream stream)
        {
            NamedPipeClientInfo clientInfo = new NamedPipeClientInfo();
            clientInfo.Stream = stream;
            clientInfo.Processor = new MessageProcessor(clientInfo);
            clientInfo.Thread = new Thread(delegate()
            {
                while (!Cancellation.IsCancellationRequested)
                {
                    try
                    {
                        clientInfo.Processor.WaitForMessage();
                    }
                    catch
                    {
                        break;
                    }
                }
                lock (_info)
                lock (_stop_lock)
                { 
                    if (_running)
                    {
                        _info.Remove(clientInfo);
                    }
                }
            });
            lock (_info)
            lock (_stop_lock)
            {
                if (_running)
                {
                    _info.Add(clientInfo);
                    clientInfo.Thread.Start();
                }
            }
        }

        public void RunServer()
        {
            _running = true;

            while (!Cancellation.IsCancellationRequested)
            {
                lock (_stop_lock)
                {
                    if (_running)
                    {
                        _listening = new NamedPipeServerStream(
                            NamedPipePath,
                            PipeDirection.InOut,
                            MaxInstances,
                            PipeTransmissionMode.Byte,
                            PipeOptions.Asynchronous | PipeOptions.WriteThrough);
                    }
                    else
                    {
                        break;
                    }
                }
                try
                {
                    _listening.WaitForConnection();
                }
                catch
                {
                    break;
                }

                OnNewConnection(_listening);
            }
        }

        public void Cancel()
        {
            lock (_stop_lock)
            {
                _running = false;
            }
            Cancellation.Cancel();
        }
    }
}
