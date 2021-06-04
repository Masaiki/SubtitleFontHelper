using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SubtitleFontHelper
{
    class FontMatcher
    {
        private Dictionary<string, FontMappingRecord> _families = new Dictionary<string, FontMappingRecord>();
        private Dictionary<string, FontFaceInfo> _fullNames = new Dictionary<string, FontFaceInfo>();
        private Dictionary<string, FontFaceInfo> _psNames = new Dictionary<string, FontFaceInfo>();

        /// <summary>
        /// Add a font face to matcher, index them by their win32 family names and full names
        /// </summary>
        /// <param name="face">The font face</param>
        /// <returns>true if a name is duplicated with other font faces</returns>
        public bool AddFontFace(FontFaceInfo face)
        {
            bool duplicated = false;
            foreach (var familyName in face.Win32FamilyName)
            {
                FontMappingRecord record;
                if (_families.TryGetValue(familyName, out record))
                {
                    record.FontFace.Add(face);
                }
                else
                {
                    record = new FontMappingRecord();
                    record.MapName = familyName;
                    record.FontFace.Add(face);
                    _families.Add(familyName, record);
                }
            }

            FontFaceInfo faceInfo;
            if (face.PostScriptName != null)
            {
                if (_psNames.TryGetValue(face.PostScriptName, out faceInfo))
                {
                    duplicated = true;
                }
                else
                {
                    _psNames.Add(face.PostScriptName, face);
                }
            }

            foreach (var fullName in face.FullName)
            {
                if (_fullNames.TryGetValue(fullName, out faceInfo))
                {
                    duplicated = true;
                }
                else
                {
                    _fullNames.Add(fullName, face);
                }
            }

            return duplicated;
        }

        public List<FontFaceInfo> FindFontFace(string faceName)
        {
            List<FontFaceInfo> faceList = new List<FontFaceInfo>();
            FontMappingRecord record;
            FontFaceInfo face;

            if (_families.TryGetValue(faceName, out record))
            {
                faceList.AddRange(record.FontFace);
            }
            else if (_fullNames.TryGetValue(faceName, out face))
            {
                faceList.Add(face);
            }
            else if (_psNames.TryGetValue(faceName, out face))
            {
                faceList.Add(face);
            }

            return faceList;
        }
    }
}
