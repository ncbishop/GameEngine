#include "ConfigFileInterface.h"
#include <fstream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

typedef std::pair<std::string, int>			sti_pair;
typedef std::pair<std::string, std::string> sts_pair;

namespace ConfigFile {

	ConfigFileInterface::ConfigFileInterface(const std::string& filename) : filename(filename), text(nullptr), modified(false), root_node(nullptr)
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::ConfigFileInterface(std::string filename)");
		logger->info("ConfigFileInterface created: {0}", filename);

		// Will either return text or throw an exception
		this->LoadXMLConfigFile(filename);
		// Then parse it into a tree
		this->ParseXML();
	}

	ConfigFileInterface::~ConfigFileInterface()
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::~ConfigFileInterface()");
		logger->info("ConfigFileInterface destroyed");
	}

	ConfigObject* ConfigFileInterface::GetConfigObject(const std::string& key) const
		// Returns non-owning raw pointer to ConfigObject
		//   Lifetime managed by ConfigFileInterface
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::GetConfigObject(const std::string& key)");
		logger->debug("Retrieve ConfigObject: \"{0}\"", key);

		// Check cache, as ConfigObjects are expensive to construct
		auto coit = this->cache.find(key);
		
		if (coit != this->cache.end())
		{
			logger->debug("Cache hit! Return stored ConfigObject*");
			// Return raw pointer to ConfigObject
			return coit->second.get();
		}
		else
		{
			logger->debug("Cache miss. Construct new ConfigObject");
			// Get the requested config_data from the XML doc
			auto config_data = this->xmlcf.first_node(key.c_str());
			logger->trace("Pointer \"{0}\" returned when looking for node \"{1}\"", static_cast<void*>(config_data), key);

			// Expensive construction (this is when the text parsing happens,
			//  very lazy!)
			auto cop = new ConfigObject(config_data);
			this->cache.emplace(std::make_pair(key, cop));

			return cop;
		}
	}

	const config_node* ConfigFileInterface::GetConfigNode(const std::string& key) const
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::GetConfigNode(std::string key)");
		logger->debug("ConfigObject configvalue retrieved: {0}", key);

		return this->xmlcf.first_node(key.c_str());
	}

	const std::string& ConfigFileInterface::GetFilename() const
	{
		return this->filename;
	}

	void ConfigFileInterface::SetConfigValue(std::string key, std::string value)
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::SetConfigValue(std::string key, std::string value)");
		logger->error("TODO FIXME function not implemented! Configvalue set: {0}\t{1}", key, value);
		
		throw ConfigFileException("Function not implemented!", key, value);

		this->modified = true;
	}

	void ConfigFileInterface::InsertConfigValue(config_node* data)
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::SetConfigValue(std::string key, std::string value)");
		logger->error("TODO FIXME function not implemented! InsertConfigValue: {0}\t{1}", data->name(), data->value());

		throw ConfigFileException("Function not implemented!", data->name(), data->value());
		
		//rapidxml::print(std::cerr, data);
		this->modified = true;
	}

	void ConfigFileInterface::LoadXMLConfigFile(std::string filename)
		// Loads contents of file into the "text" buffer inside the ConfigFileInterface 
		//	object. Don't do this if the file is super big, because I never delete it
		//	until the ConfigFileInterface object goes out of scope.
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigFileInterface::LoadXMLConfigFile(std::string filename)");

		std::fstream cf(filename);

		if (cf.is_open())
		{
			logger->info("File opened successfully: {0}", filename);
			this->filename = filename;

			// How long is the file? (in chars)
			// TODO what happens if these are wide chars?
			cf.seekg(0, cf.end);
			int length = cf.tellg();
			cf.seekg(0, cf.beg);

			// Make buffer to handle config data, RapidXML requires the text to persist
			// Also "read" does not zero terminate, so add that at the end
			this->text = std::make_unique<char[]>(length + 1);
			cf.read(text.get(), length);
			text[length] = '\0';

			if (cf.eof())
			{
				logger->debug("All {0} characters read successfully!", length);
				//std::cout << text.get() << std::endl;
			}
			else
			{
				logger->error("Error: only {0} characters read, should have been {1}", cf.gcount(), length);
				throw ConfigFileException("Unable to read contents of config file.", "", filename);
			}
			cf.close();
		}
		else
		{
			logger->error("Cannot find file: {0}", filename);
			throw ConfigFileException("Cannot find or open config file.", "", filename);
		}

		return;
	}

	void ConfigFileInterface::ParseXML()
	{
		// therefore no need to check for null "text" pointer
		this->xmlcf.parse<0>(this->text.get());

		// Get root node
		//this->root_node = this->xmlcf.first_node("root");
		this->root_node = this->xmlcf.first_node();
		if (root_node == nullptr)
		{
			auto logger = spdlog::get("EngineLogger");
			logger->error("Config file format error - no root node");
			throw ConfigFileException("Config file format error - no root node", "", filename);
		}
	}

	ConfigObject::ConfigObject(config_node* config_data)
	{
		auto logger = spdlog::get("EngineLogger");
		logger->trace("ConfigObject::ConfigObject(config_node*)");

		this->name = config_data->name();
		this->data = config_data->value();
		logger->debug("ConfigObject name \"{0}\" with data \"{1}\"", this->name, this->data);

		for (auto attr = config_data->first_attribute(); attr != nullptr; attr = attr->next_attribute())
		{
			this->attrib_map[attr->name()] = attr->value();
			logger->debug("ConfigObject attribute \"{0}\" with value \"{1}\"", attr->name(), attr->value());
		}

		auto cn = config_data->first_node();
		while (cn != nullptr)
		{
			// Use pointer to child config_data to create a ConfigObject in this->children vector
			this->children.emplace_back(cn);
			// Also store a map from name to index
			this->children_byname.emplace(std::make_pair<std::string, int>(std::string(cn->name()), this->children.size()-1));
			cn = cn->next_sibling();
		}
	}

	const std::string& ConfigObject::GetAttribute(const std::string& key) const
	{
		return this->attrib_map.at(key);
	}

	void ConfigObject::SetAttribute(const std::string& key, const std::string& val)
	{
		this->attrib_map.emplace(std::make_pair(key, val));
	}
	
} // end namespace ConfigFile