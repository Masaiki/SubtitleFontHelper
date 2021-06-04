using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SubtitleFontHelper
{
    class MessageProcessor
    {
        public NamedPipeClientInfo ClientInfo { get; private set; }

        public MessageProcessor(NamedPipeClientInfo clientInfo)
        {
            ClientInfo = clientInfo;
        }

        public void WaitForMessage()
        {
            using (BinaryReader reader = new BinaryReader(ClientInfo.Stream, Encoding.UTF8, true)) 
            {
                UInt32 length = reader.ReadUInt32();
                byte[] buffer = reader.ReadBytes((int)length);
                Message msg = Message.Parser.ParseFrom(buffer);

                switch(msg.Type){
                    case MessageType.IndicationAttach:
                        OnAttachIndication(msg.AttachIndication);
                        break;
                    case MessageType.RequestFontQuery:
                        OnFontQueryRequest(msg.FontQueryRequest);
                        break;
                    default:
                        break;
                }
            }
        }

        private void OnAttachIndication(AttachIndication msg)
        {
            ClientInfo.ProcessId = msg.ProcessId;
        }

        private void OnFontQueryRequest(FontQueryRequest msg)
        {
            List<FontFaceInfo> faceList = GlobalContext.FontMatcher.FindFontFace(msg.FaceName);
            Message respmsg = new Message();
            respmsg.Type = MessageType.ResponseFontQuery;
            respmsg.FontQueryResponse = new FontQueryResponse();
            foreach (var face in faceList)
            {
                respmsg.FontQueryResponse.FullPath.Add(face.FileName);
            }
            SendMessage(respmsg);
        }

        private void SendMessage(Message msg)
        {
            UInt32 length = (uint)msg.CalculateSize();
            using (MemoryStream memory = new MemoryStream())
            using (BinaryWriter writer = new BinaryWriter(memory))
            {
                writer.Write(length);
                writer.Flush();
                using (CodedOutputStream codedOs = new CodedOutputStream(memory))
                {
                    msg.WriteTo(codedOs);
                    codedOs.Flush();
                    byte[] buffer = memory.ToArray();
                    ClientInfo.Stream.Write(buffer, 0, buffer.Length);
                }
            }
        }
    }
}
