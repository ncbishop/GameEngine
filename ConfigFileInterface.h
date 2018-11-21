#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "rapidxml-1.13\rapidxml.hpp"

namespace ConfigFile {

	using config_node = rapidxml::xml_node<char>;
	using config_attribute = rapidxml::xml_attribute<char>;
	class ConfigObject;

	class ConfigFileInterface
		// Converts an XML configuration file into ConfigObjects
		// TODO enable writing configuration files!
		// Set and insert functions work on in-memory objects, but currently nothing writes to files.
	{
	public:
		ConfigFileInterface(const std::string& filename);
		~ConfigFileInterface();

		ConfigObject* GetConfigObject(const std::string& key) const;
		const config_node* GetConfigNode(const std::string& key) const;
		
		const std::string& GetFilename() const;
		
		void SetConfigValue(std::string key, std::string value);
		void InsertConfigValue(config_node* data);

	private:

		void LoadXMLConfigFile(std::string filename);
		void ParseXML();

		std::string filename;
		std::unique_ptr<char[]> text;

		rapidxml::xml_document<char> xmlcf;
		config_node* root_node;

		mutable std::map<std::string, std::unique_ptr<ConfigObject>> cache;

		bool modified;
	};

	class ConfigObject
		// Converts an XML node structure into a set of maps.
		//	Actual values are retrieved from the maps using string based keywords.
		//	This is basically a crude Python Dict that is type-aware...
		//
		// TODO enable serializing a configuration back into an XML node structure.
	{
	public:
		ConfigObject(config_node* config_data);
		~ConfigObject() = default;

		virtual config_node* Serialize() { return nullptr; }

		const std::string& GetName() const { return this->name; }
		const std::string& GetAttribute(const std::string& key) const;
		const std::string& GetData() const { return this->data; }
		const std::vector<ConfigObject>& GetChildren() const { return this->children; }

	protected:
		// May make these public once we enable setting/saving configurations
		// TODO make these public
		void SetAttribute(const std::string& key, const std::string& val);
		void AddChild(ConfigObject co);

	private:
		// Contains conversion maps per type
		// Right now you have to know the type you are expecting
		std::string name;

		std::map<std::string, std::string> attrib_map;
		std::string data;
		std::vector<ConfigObject> children;
		std::multimap<std::string, int> children_byname;
	};

	struct ConfigFileException
	{
	public:
		ConfigFileException(const std::string message, const std::string path, const std::string name):
			message(message), path(path), name(name)
		{}

		std::string message;
		std::string path;
		std::string name;
	};

} // end namespace ConfigFile