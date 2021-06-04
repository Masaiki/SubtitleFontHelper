using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace SubtitleFontHelper
{
    [Serializable]
    [XmlRoot("FontFaceCollection")]
    public class FontFaceCollection
    {

        [XmlElement("FontFace")] public List<FontFaceInfo> FontFace { get; set; } = new List<FontFaceInfo>();

        public static FontFaceCollection ParseStream(Stream stream)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(FontFaceCollection));
            XmlReaderSettings settings = new XmlReaderSettings();
            settings.CheckCharacters = false;
            XmlReader reader = XmlTextReader.Create(stream, settings);
            FontFaceCollection ret=serializer.Deserialize(reader) as FontFaceCollection;
            return ret;
        }

        public void WriteStream(Stream stream)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(FontFaceCollection));
            serializer.Serialize(stream, this);
        }
    }

    [Serializable]
    public class FontFaceInfo
    {

        //[XmlAttribute("Weight")] public int Weight { get; set; } = 0;

        //[XmlAttribute("Italic")] public bool Italic { get; set; } = false;

        [XmlElement("FileName")] public string FileName { get; set; }

        [XmlElement("Index")] public int Index { get; set; } = 0;

        [XmlElement("FullName")] public List<string> FullName { get; set; } = new List<string>();

        [XmlElement("Win32FamilyName")] public List<string> Win32FamilyName { get; set; } = new List<string>();

        //[XmlElement("FamilyName")] public List<string> FamilyName { get; set; } = new List<string>();

        [XmlElement("PostScriptName")] public string PostScriptName { get; set; }

        ///// <summary>
        ///// The typographical font family the face belongs to
        ///// </summary>
        //[XmlIgnore] public FontFamilyInfo FontFamily { get; set; }
    }
}
