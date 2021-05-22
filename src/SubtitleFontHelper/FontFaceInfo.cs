using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace SubtitleFontHelper
{
    [Serializable]
    class FontFaceInfo
    {
        [XmlAttribute("Index")] public int Index { get; set; } = 0;

        //[XmlAttribute("Weight")] public int Weight { get; set; } = 0;

        //[XmlAttribute("Italic")] public bool Italic { get; set; } = false;

        [XmlElement("FileName")] public string FileName { get; set; }

        [XmlElement("FullName")] public List<string> FullName { get; set; } = new List<string>();

        [XmlElement("Win32FamilyName")] public List<string> Win32FamilyName { get; set; } = new List<string>();

        [XmlElement("FamilyName")] public List<string> FamilyName { get; set; } = new List<string>();

        [XmlElement("PostScriptName")] public string PostScriptName { get; set; }

        /// <summary>
        /// The typographical font family the face belongs to
        /// </summary>
        [XmlIgnore] public FontFamilyInfo FontFamily { get; set; }
    }
}
