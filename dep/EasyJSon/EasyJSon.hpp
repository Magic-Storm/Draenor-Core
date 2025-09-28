/// Copyright 2015 hardcpp
/// https://github.com/hardcpp/EasyJSon
#pragma once
#ifndef EASYJSON_HPP_INCLUDED
#define EASYJSON_HPP_INCLUDED

#include <sstream>
#include <string>
#include <map>
#include <type_traits>

namespace EasyJSon
{

    /// JSon node element type
    enum NodeType
    {
        NT_INDEXED_ARRAY,
        NT_ARRAY,
        NT_DATA,
        NT_DATA_INT
    };

    /// JSon node
    template <class StringAllocator> class Node
    {
        public:
            /// @brief Get node type
            /// @return Type of current node
            const NodeType & Type()
            {
                return m_Type;
            }
            /// Sub node accessors
            /// @param p_Name : Index name of child node
            /// @return The node, or null filled node
            Node<StringAllocator> & operator[](const StringAllocator & p_Name)
            {
                m_Type = NT_INDEXED_ARRAY;

                if (m_ChildNodes.find(p_Name) == m_ChildNodes.end())
                    m_ChildNodes[p_Name] = Node();

                return m_ChildNodes[p_Name];
            }
            /// Sub node accessors
            /// @param p_ID : Index of child node
            /// @return The node, or null filled node
            Node<StringAllocator> & operator[](const size_t & p_ID);
            /// Get sub node count if type == array || indexed_array
            /// @return Child count
            const size_t Size()
            {
                return m_ChildNodes.size();
            }

            /// Get data
            /// @return Data
            const StringAllocator & GetData()
            {
                return m_Data;
            }
            /// Set data
            /// @param p_Data : New data str
            /// @param p_IsArithmetic : Is numeric value
            inline void SetData(const StringAllocator & p_Data, bool p_IsArithmetic = false)
            {
                m_Type = p_IsArithmetic ? NT_DATA_INT : NT_DATA;
                m_Data = p_Data;
            }

            /// Get child nodes
            /// @return Map of childs
            const std::map<StringAllocator, Node<StringAllocator>> & GetChilds()
            {
                return m_ChildNodes;
            }

            /// Serialize
            /// @param p_Pretty : Pretty format
            /// @param p_Level : Recursive level
            /// @return String
            template<class StringAllocatorStream> inline StringAllocator Serialize(bool p_Pretty, int p_Level = 0);

            /// Data editor operator
            template<typename T> inline Node & operator=(T p_Data);

        private:
            /// Node type
            NodeType m_Type;
            /// Node data
            StringAllocator m_Data;
            /// Child nodes
            std::map<StringAllocator, Node<StringAllocator>> m_ChildNodes;

    };

    /// @brief Sub node accessors
    /// @param p_ID : Index of child node
    /// @return The node, or null filled node
    template <> inline Node<std::string> & Node<std::string>::operator[](const size_t & p_ID)
    {
        m_Type = NT_ARRAY;

        std::string l_KeyName = std::to_string((unsigned long)p_ID);

        if (m_ChildNodes.find(l_KeyName) == m_ChildNodes.end())
            m_ChildNodes[l_KeyName] = Node();

        return m_ChildNodes[l_KeyName];
    }
    /// @brief Sub node accessors
    /// @param p_ID : Index of child node
    /// @return The node, or null filled node
    template <> inline Node<std::wstring> & Node<std::wstring>::operator[](const size_t & p_ID)
    {
        m_Type = NT_ARRAY;

        std::wstring l_KeyName = std::to_wstring((unsigned long)p_ID);

        if (m_ChildNodes.find(l_KeyName) == m_ChildNodes.end())
            m_ChildNodes[l_KeyName] = Node();

        return m_ChildNodes[l_KeyName];
    }

    /// @brief Serialize
    /// @param p_Pretty : Pretty format
    /// @param p_Level : Recursive level
    /// @return String
    template <class StringAllocator>
    template <class StringAllocatorStream> inline StringAllocator Node<StringAllocator>::Serialize(bool p_Pretty, int p_Level)
    {
        if (m_Type == NT_DATA)
        {
            return StringAllocator("\"" + m_Data + "\"");
        }
        else if (m_Type == NT_DATA_INT)
        {
            return StringAllocator(m_Data);
        }
        else if (m_Type == NT_ARRAY)
        {
            StringAllocatorStream l_Out;

            if (p_Pretty && p_Level != 0)
                l_Out << "\n";

            l_Out << std::string(p_Level, '\t') << "[" << (p_Pretty ? "\n" : "");

            for (typename std::map<StringAllocator, Node<StringAllocator>>::iterator l_It = m_ChildNodes.begin(); l_It != m_ChildNodes.end(); l_It++)
            {
                if (l_It != m_ChildNodes.begin())
                    l_Out << ",";

                l_Out << l_It->second.template Serialize<StringAllocatorStream>(p_Pretty, p_Level + 1);
            }

             if (p_Pretty)
                 l_Out << "\n";

            l_Out << std::string(p_Level, '\t') << "]";

            return l_Out.str();
        }
        else
        {
            StringAllocatorStream l_Out;

            if (p_Pretty && p_Level != 0)
                l_Out << "\n";

            l_Out << std::string(p_Level, '\t') << "{" << (p_Pretty ? "\n" : "");

            for (typename std::map<StringAllocator, Node<StringAllocator>>::iterator l_It = m_ChildNodes.begin(); l_It != m_ChildNodes.end(); l_It++)
            {
                if (l_It != m_ChildNodes.begin())
                    l_Out << ",\n";

                l_Out << std::string(p_Level + 1, '\t') << "\"" << l_It->first << "\": " << l_It->second.template Serialize<StringAllocatorStream>(p_Pretty, p_Level + 1);
            }

            if (p_Pretty)
                l_Out << "\n";

            l_Out << std::string(p_Level, '\t') << "}";

            return l_Out.str();
        }

        return StringAllocator("");
    }
    /// @brief Serialize
    /// @param p_Pretty : Pretty format
    /// @param p_Level : Recursive level
    /// @return String
    template <>
    template <class StringAllocatorStream> inline std::wstring Node<std::wstring>::Serialize(bool p_Pretty, int p_Level)
    {
        if (m_Type == NT_DATA)
        {
            return std::wstring(L"\"" + m_Data + L"\"");
        }
        else if (m_Type == NT_DATA_INT)
        {
            return std::wstring(m_Data);
        }
        else if (m_Type == NT_ARRAY)
        {
            StringAllocatorStream l_Out;

            if (p_Pretty && p_Level != 0)
                l_Out << L"\n";

            l_Out << std::wstring(p_Level, L'\t') << L"[" << (p_Pretty ? L"\n" : L"");

            for (std::map<std::wstring, Node<std::wstring>>::iterator l_It = m_ChildNodes.begin(); l_It != m_ChildNodes.end(); l_It++)
            {
                if (l_It != m_ChildNodes.begin())
                    l_Out << L",";

                l_Out << l_It->second.Serialize<StringAllocatorStream>(p_Pretty, p_Level + 1);
            }

            if (p_Pretty)
                l_Out << L"\n";

            l_Out << std::wstring(p_Level, L'\t') << L"]";

            return l_Out.str();
        }
        else
        {
            StringAllocatorStream l_Out;

            if (p_Pretty && p_Level != 0)
                l_Out << L"\n";

            l_Out << std::wstring(p_Level, L'\t') << L"{" << (p_Pretty ? L"\n" : L"");

            for (std::map<std::wstring, Node<std::wstring>>::iterator l_It = m_ChildNodes.begin(); l_It != m_ChildNodes.end(); l_It++)
            {
                if (l_It != m_ChildNodes.begin())
                    l_Out << L",\n";

                l_Out << std::wstring(p_Level + 1, L'\t') << L"\"" << l_It->first << L"\": " << l_It->second.Serialize<StringAllocatorStream>(p_Pretty, p_Level + 1);
            }

            if (p_Pretty)
                l_Out << L"\n";

            l_Out << std::wstring(p_Level, L'\t') << L"}";

            return l_Out.str();
        }

        return std::wstring(L"");
    }

    /// Data editor operator
    template<>
    template<typename T> inline Node<std::string> & Node<std::string>::operator=(T p_Data)
    {
        SetData(std::to_string(p_Data), std::is_arithmetic<T>::value);
        return *this;
    }
    /// Data editor operator
    template<>
    template<typename T> inline Node<std::wstring> & Node<std::wstring>::operator=(T p_Data)
    {
        SetData(std::to_wstring(p_Data), std::is_arithmetic<T>::value);
        return *this;
    }
    /// Data editor operator
    template<>
    template<> inline Node<std::string> & Node<std::string>::operator=(const char * p_Data)
    {
        SetData(p_Data);
        return *this;
    }
    /// Data editor operator
    template<>
    template<> inline Node<std::string> & Node<std::string>::operator=(std::string p_Data)
    {
        SetData(p_Data);
        return *this;
    }
    /// Data editor operator
    template<>
    template<> inline Node<std::wstring> & Node<std::wstring>::operator=(const wchar_t * p_Data)
    {
        SetData(p_Data);
        return *this;
    }
    /// Data editor operator
    template<>
    template<> inline Node<std::wstring> & Node<std::wstring>::operator=(std::wstring p_Data)
    {
        SetData(p_Data);
        return *this;
    }

}   ///< namespace EasyJSon

#endif  ///< EASYJSON_HPP_INCLUDED