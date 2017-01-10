import os
import ConfigParser

class ConfigFileHandler(object):
  '''This class handles a config file.

  The config file contains the attributes of a parsed workspace. The config file
  format is the following:

  [workspace.name1]
  key1 = value1
  key2 = value2

  [workspace.name2]
  key3 = value3
  key4 = value4
  key5 = value5

  This class supposes this file format. The file may optionally contain lines
  starting with hashmark, but these are lost after the first call of save()
  function.'''

  def __init__(self, filename):
    '''Open the config file and load its content.'''

    self.__filename = filename
    self.__configParser = ConfigParser.SafeConfigParser()
    self.__load()

  def add(self, name):
    '''Add a workspace to the config list with no attribbutes. Addition takes
    effect when save() is called. If workspace already exists with the given
    name then it will be overwritten.'''
    section_name = self.__ws_to_section(name)
    self.__configParser.add_section(section_name)

  def remove(self, name):
    '''Remove a workspace from the config list. Remove takes effect when save()
    is called.'''
    section_name = self.__ws_to_section(name)
    self.__configParser.remove_section(section_name)

  def exists(self, name):
    '''Returns true if "name" is an existing workspace.'''
    section_name = self.__ws_to_section(name)
    return section_name in self.__configParser.sections()

  def rename(self, from_name, to_name):
    '''Rename a workspace. Rename takes effect when save() is called.'''

    section_from = self.__ws_to_section(from_name)
    section_to = self.__ws_to_section(to_name)
    self.__configParser.remove_section(section_to)
    self.__configParser.add_section(section_to)
    for name, value in self.__configParser.items(section_from):
      self.__configParser.set(section_to, name, value)
    self.__configParser.remove_section(section_from)

  def set_attribute(self, name, key, value):
    '''Set an attribute of the given workspace.'''
    section_name = self.__ws_to_section(name)
    self.__configParser.set(section_name, key, value)

  def get_attribute(self, name, key):
    '''Return the given attribute of a workspace if the attribute exists or
    return None.'''
    section_name = self.__ws_to_section(name)
    if not self.__configParser.has_section(section_name) or \
       not self.__configParser.has_option(section_name, key):
      return None

    return self.__configParser.get(section_name, key)

  def get_attributes(self, name):
    '''Returns the attributes of a workspace if the workspace exists or
    return None.'''
    section_name = self.__ws_to_section(name)
    if not self.__configParser.has_section(section_name):
      return []

    return self.__configParser.options(section_name)

  def save(self):
    '''Persists the modifications of the config file.'''
    with open(self.__filename, 'wb') as f:
      self.__configParser.write(f)

  def names(self):
    '''Returns the list of workspace names from the config file.'''
    result = []
    for section in self.__configParser.sections():
      if section.find('workspace.') == 0:
        result.append(section[10:])
    return result

  def __load(self):
    '''Loads the content of the workspace file to the inner representation.'''
    if not os.path.isfile(self.__filename):
      return

    with open(self.__filename) as f:
      self.__configParser.readfp(f)

  def __ws_to_section(self, name):
    return 'workspace.' + name
