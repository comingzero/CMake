#include "cmTestConfigure.h"
#include "cmTestConfigureEscape.h"
#include "cmTestGeneratedHeader.h"
#include "cmVersion.h"
#include "ExtraSources/file1.h"
#include "file2.h"
#include "sharedFile.h"
extern "C" {
#include "testConly.h"
}
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmDynamicLoader.h"
#include "cmSystemTools.h"
#include "cmOrderLinkDirectories.h"

int cm_passed = 0;
int cm_failed = 0;

#ifndef CMAKE_IS_REALLY_FUN
This is a problem. Looks like ADD_DEFINITIONS and REMOVE_DEFINITIONS does not work
#endif
// Here is a stupid function that tries to use std::string methods
// so that the dec cxx compiler will instantiate the stuff that
// we are using from the CMakeLib library....
bool TestLibraryOrder(bool shouldFail)
{ 
  std::string Adir = std::string(BINARY_DIR) + std::string("/A");
  std::string Bdir = std::string(BINARY_DIR) + std::string("/B");
  std::string Cdir = std::string(BINARY_DIR) + std::string("/C");
  
  if(!shouldFail)
    {
    std::string rm = Bdir;
    rm += "/libA.a";
    cmSystemTools::RemoveFile(rm.c_str());
    }
  cmTarget target;
  target.AddLinkDirectory(Adir.c_str());
  target.AddLinkDirectory(Bdir.c_str());
  target.AddLinkDirectory(Cdir.c_str());
  target.AddLinkDirectory("/lib/extra/stuff");
  
  Adir += "/libA.a";
  Bdir += "/libB.a";
  Cdir += "/libC.a";
  
  target.AddLinkLibrary(Adir.c_str(), cmTarget::GENERAL);
  target.AddLinkLibrary(Bdir.c_str(), cmTarget::GENERAL);
  target.AddLinkLibrary(Cdir.c_str(), cmTarget::GENERAL);
  target.AddLinkLibrary("-lm", cmTarget::GENERAL);
  std::vector<cmStdString> sortedpaths;
  std::vector<cmStdString> linkItems;
  cmOrderLinkDirectories orderLibs;
  orderLibs.DebugOn();
  orderLibs.AddLinkExtension(".so");
  orderLibs.AddLinkExtension(".a");
  orderLibs.SetLinkInformation(target, cmTarget::GENERAL, "A");
  bool ret = orderLibs.DetermineLibraryPathOrder();
  orderLibs.GetLinkerInformation(sortedpaths, linkItems);
  std::cout << "Sorted Link Paths:\n";
  for(std::vector<cmStdString>::iterator i = sortedpaths.begin();
      i != sortedpaths.end(); ++i)
    {
    std::cout << *i << "\n";
    }
  std::cout << "Link Items: \n";
  for(std::vector<cmStdString>::iterator i = linkItems.begin();
      i != linkItems.end(); ++i)
    {
    std::cout << *i << "\n";
    }
  if(!(linkItems[0] == "A" && 
       linkItems[1] == "B" && 
       linkItems[2] == "C" && 
       linkItems[3] == "-lm" ))
    {
    std::cout << "fail because link items should be A B C -lm and the are not\n";
    return shouldFail;
    }
  
     
  // if this is not the fail test then the order should be f B C A
  if(!shouldFail)
    {
    char order[5];
    order[4] = 0;
    for(int i =0; i < 4; ++i)
      {
      order[i] = sortedpaths[i][sortedpaths[i].size()-1];
      }
    if(!(strcmp(order, "fBCA") == 0 || strcmp(order, "BCAf") == 0))
      {
      std::cout << "fail because order should be /lib/extra/stuff B C A and it is not\n";
      return false;
      }
    }
  return ret;
}

void ForceStringUse()
{
  std::vector<std::string> v;
  std::vector<std::string> v2;
  v = v2;
  std::string cachetest = CACHE_TEST_VAR_INTERNAL;
  v.push_back(cachetest);
  v2 = v;
  std::string x(5,'x');  
  char buff[5];
  x.copy(buff, 1, 0);
  x[0] = 'a';
  std::string::size_type pos = 0;
  x.replace(pos, pos, pos, 'x');
  std::string copy = cachetest;
  cachetest.find("bar");
  cachetest.rfind("bar");
  copy.append(cachetest);
  copy = cachetest.substr(0, cachetest.size());
}

// ======================================================================

void cmFailed(const char* Message, const char* m2= "", const char* m3 = "")
{
  std::cout << "FAILED: " << Message << m2 << m3 << "\n"; 
  cm_failed++;
}

// ======================================================================

void cmPassed(const char* Message, const char* m2="")
{
  std::cout << "Passed: " << Message << m2 << "\n"; 
  cm_passed++;
}

// ======================================================================

void TestAndRemoveFile(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    cmFailed("Could not find file: ", filename);
    }
  else
    {
    if (!cmSystemTools::RemoveFile(filename))
      {
      cmFailed("Unable to remove file. It does not imply that this test failed, but it *will* be corrupted thereafter if this file is not removed: ", filename);
      }
    else
      {
      cmPassed("Find and remove file: ", filename);
      }
    }
}

// ======================================================================

void TestDir(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    cmFailed("Could not find dir: ", filename);
    }
  else
    {
    if (!cmSystemTools::FileIsDirectory(filename))
      {
      cmFailed("Unable to check if file is a directory: ", filename);
      }
    else
      {
      cmPassed("Find dir: ", filename);
      }
    }
}

// ======================================================================

int main()
{
  std::string lib = BINARY_DIR;
  lib += "/bin/";
#ifdef  CMAKE_INTDIR
  lib += CMAKE_INTDIR;
  lib += "/";
#endif
  std::string exe = lib;

  // Test a single character executable to test a: in makefiles
  exe += "A";
  exe += cmSystemTools::GetExecutableExtension();
  int ret;
  std::string errorMessage; 
  exe = cmSystemTools::ConvertToRunCommandPath(exe.c_str());
  if(cmSystemTools::RunSingleCommand(exe.c_str(), 0, &ret))
    {
    if(ret != 10)
      {
      errorMessage += exe;
      errorMessage += " did not return 10";
      }
    }
  else
    {
    errorMessage += exe;
    errorMessage += ": failed to run.";
    }
  if(errorMessage.size())
    {
    cmFailed(errorMessage.c_str());
    }
  else
    {
    cmPassed("run Single Character executable A returned 10 as expected.");
    }
  
  lib += cmDynamicLoader::LibPrefix();
  lib += "CMakeTestModule";
  lib += cmDynamicLoader::LibExtension();
  cmLibHandle handle = cmDynamicLoader::OpenLibrary(lib.c_str());
  if(!handle)
    {
    std::string err = "Can not open CMakeTestModule:\n";
    err += lib;
    cmFailed(err.c_str());
    }
  else
    {
    cmDynamicLoaderFunction fun = 
    cmDynamicLoader::GetSymbolAddress(handle, "ModuleFunction"); 
    if(!fun)
      {
      fun = cmDynamicLoader::GetSymbolAddress(handle, "_ModuleFunction");
      }
    typedef int (*TEST_FUNCTION)();
    TEST_FUNCTION testFun = (TEST_FUNCTION)fun;
    if(!testFun)
      {
      cmFailed("Could not find symbol ModuleFunction in library ");
      }
    else
      {
        int ret = (*testFun)();
        if(!ret)
          {
          cmFailed("ModuleFunction call did not return valid return.");
          }
        cmPassed("Module loaded and ModuleFunction called correctly.");
      }
    }
  if(sharedFunction() != 1)
    {
    cmFailed("Call to sharedFunction from shared library failed.");
    }
  else
    {
    cmPassed("Call to sharedFunction from shared library worked.");
    }
  if(CsharedFunction() != 1)
    {
    cmFailed("Call to C sharedFunction from shared library failed.");
    }
  else
    {
    cmPassed("Call to C sharedFunction from shared library worked.");
    }
  
  if(file1() != 1)
    {
    cmFailed("Call to file1 function from library failed.");
    }
  else
    {
    cmPassed("Call to file1 function returned 1.");
    }

  if(file2() != 1)
    {
    cmFailed("Call to file2 function from library failed.");
    }
  else
    {
    cmPassed("Call to file2 function returned 1.");
    }
#ifndef TEST_CXX_FLAGS
  cmFailed("CMake CMAKE_CXX_FLAGS is not being passed to the compiler!");
#else
  cmPassed("CMake CMAKE_CXX_FLAGS is being passed to the compiler.");
#endif
  // ----------------------------------------------------------------------
  // Test ADD_DEFINITIONS

#ifndef CMAKE_IS_FUN
  cmFailed("CMake is not fun, so it is broken and should be fixed.");
#else
  cmPassed("CMAKE_IS_FUN is defined.");
#endif
  
#if defined(CMAKE_ARGV1) && defined(CMAKE_ARGV2) && defined(CMAKE_ARGV3) && defined(CMAKE_ARGV4) 
  cmPassed("Variable args for MACROs are working.");
#else
  cmFailed("Variable args for MACROs are failing.");
#endif

  // ----------------------------------------------------------------------
  // Test SET, VARIABLE_REQUIRES

#ifdef SHOULD_NOT_BE_DEFINED
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED is defined.");
#endif
  
#ifndef ONE_VAR
  cmFailed("cmakedefine is broken, ONE_VAR is not defined.");
#else
  cmPassed("ONE_VAR is defined.");
#endif
  
#ifndef ONE_VAR_IS_DEFINED
  cmFailed("cmakedefine, SET or VARIABLE_REQUIRES is broken, "
         "ONE_VAR_IS_DEFINED is not defined.");
#else
  cmPassed("ONE_VAR_IS_DEFINED is defined.");
#endif
  
#ifdef ZERO_VAR
  cmFailed("cmakedefine is broken, ZERO_VAR is defined.");
#else
  cmPassed("ZERO_VAR is not defined.");
#endif
  
#ifndef STRING_VAR
  cmFailed("the CONFIGURE_FILE command is broken, STRING_VAR is not defined.");
#else
  if(strcmp(STRING_VAR, "CMake is great") != 0)
    {
    cmFailed("the SET or CONFIGURE_FILE command is broken. STRING_VAR == ", 
           STRING_VAR);
    }
  else
    {
    cmPassed("STRING_VAR == ", STRING_VAR);
    }
#endif

  // ----------------------------------------------------------------------
  // Test various IF/ELSE combinations

#ifdef SHOULD_NOT_BE_DEFINED_NOT
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_NOT is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_NOT is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_NOT
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_NOT is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_NOT is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_NOT2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_NOT2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_NOT2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_NOT2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_NOT2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_NOT2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_AND
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_AND is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_AND is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_AND
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_AND is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_AND is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_AND2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_AND2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_AND2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_AND2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_AND2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_AND2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_OR
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_OR is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_OR is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_OR
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_OR is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_OR is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_OR2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_OR2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_OR2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_OR2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_OR2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_OR2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_MATCHES
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_MATCHES is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_MATCHES is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_MATCHES
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_MATCHES is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_MATCHES is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_MATCHES2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_MATCHES2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_MATCHES2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_MATCHES2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_MATCHES2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_MATCHES2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_COMMAND
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_COMMAND is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_COMMAND is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_COMMAND
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_COMMAND is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_COMMAND is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_COMMAND2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_COMMAND2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_COMMAND2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_COMMAND2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_COMMAND2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_COMMAND2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_EXISTS
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_EXISTS is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_EXISTS is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_EXISTS
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_EXISTS is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_EXISTS is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_EXISTS2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_EXISTS2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_EXISTS2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_EXISTS2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_EXISTS2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_EXISTS2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_LESS
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_LESS is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_LESS is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_LESS
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_LESS is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_LESS is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_LESS2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_LESS2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_LESS2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_LESS2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_LESS2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_LESS2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_GREATER
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_GREATER is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_GREATER is not defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_EQUAL
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_EQUAL is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_EQUAL is not defined.");
#endif

#ifndef SHOULD_BE_DEFINED_EQUAL
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_EQUAL is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_EQUAL is defined.");
#endif

#ifndef SHOULD_BE_DEFINED_GREATER
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_GREATER is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_GREATER is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_GREATER2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_GREATER2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_GREATER2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_GREATER2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_GREATER2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_GREATER2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_STRLESS
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_STRLESS is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_STRLESS is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_STRLESS
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_STRLESS is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_STRLESS is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_STRLESS2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_STRLESS2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_STRLESS2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_STRLESS2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_STRLESS2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_STRLESS2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_STRGREATER
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_STRGREATER is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_STRGREATER is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_STRGREATER
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_STRGREATER is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_STRGREATER is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_STRGREATER2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_STRGREATER2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_STRGREATER2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_STRGREATER2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_STRGREATER2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_STRGREATER2 is defined.");
#endif
  
  // ----------------------------------------------------------------------
  // Test FOREACH

#ifndef FOREACH_VAR1
  cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR1 is not defined.");
#else
  if(strcmp(FOREACH_VAR1, "VALUE1") != 0)
    {
    cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR1 == ", FOREACH_VAR1);
    }
  else
    {
    cmPassed("FOREACH_VAR1 == ", FOREACH_VAR1);
    }
#endif

#ifndef FOREACH_VAR2
  cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR2 is not defined.");
#else
  if(strcmp(FOREACH_VAR2, "VALUE2") != 0)
    {
    cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR2 == ", FOREACH_VAR2);
    }
  else
    {
    cmPassed("FOREACH_VAR2 == ", FOREACH_VAR2);
    }
#endif

#ifndef FOREACH_CONCAT
  cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_CONCAT is not defined.");
#else
  if(strcmp(FOREACH_CONCAT, "abcdefg") != 0)
    {
    cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_CONCAT == ", FOREACH_CONCAT);
    }
  else
    {
    cmPassed("FOREACH_CONCAT == ", FOREACH_CONCAT);
    }
#endif

  // ----------------------------------------------------------------------
  // Test FIND_FILE, FIND_PATH and various GET_FILENAME_COMPONENT combinations

#ifndef FILENAME_VAR_PATH_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_PATH_NAME is not defined.");
#else
  if((strcmp(FILENAME_VAR_PATH_NAME, "Complex") == 0) ||
     (strcmp(FILENAME_VAR_PATH_NAME, "ComplexOneConfig") == 0) ||
     (strcmp(FILENAME_VAR_PATH_NAME, "ComplexRelativePaths") == 0))
    {
    cmPassed("FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
  else
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
#endif

#ifndef FILENAME_VAR_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME, "VarTests.cmake") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
  else
    {
    cmPassed("FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
#endif

#ifndef FILENAME_VAR_EXT
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_EXT is not defined.");
#else
  if(strcmp(FILENAME_VAR_EXT, ".cmake") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
  else
    {
    cmPassed("FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
#endif

#ifndef FILENAME_VAR_NAME_WE
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME_WE is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME_WE, "VarTests") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
  else
    {
    cmPassed("FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
#endif

#ifndef PATH_VAR_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "PATH_VAR_NAME is not defined.");
#else
  if((strcmp(PATH_VAR_NAME, "Complex") == 0) ||
     (strcmp(PATH_VAR_NAME, "ComplexOneConfig") == 0) ||
     (strcmp(PATH_VAR_NAME, "ComplexRelativePaths") == 0))
    {
    cmPassed("PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
  else
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
#endif

  // ----------------------------------------------------------------------
  // Test LOAD_CACHE

#ifndef CACHE_TEST_VAR1
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR1 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR1, "foo") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
#endif

#ifndef CACHE_TEST_VAR2
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR2 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR2, "bar") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
#endif

#ifndef CACHE_TEST_VAR3
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR3 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR3, "1") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
#endif

#ifdef CACHE_TEST_VAR_EXCLUDED
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command or cmakedefine is broken, "
         "CACHE_TEST_VAR_EXCLUDED is defined (should not have been loaded).");
#else
  cmPassed("CACHE_TEST_VAR_EXCLUDED is not defined.");
#endif

#ifndef CACHE_TEST_VAR_INTERNAL
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR_INTERNAL is not defined.");
#else
  std::string cachetest = CACHE_TEST_VAR_INTERNAL;
  if(cachetest != "bar")
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR_INTERNAL == ", CACHE_TEST_VAR_INTERNAL);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR_INTERNAL == ", CACHE_TEST_VAR_INTERNAL);
    }
#endif

  // ----------------------------------------------------------------------
  // Some pre-build/pre-link/post-build custom-commands have been
  // attached to the lib (see Library/).
  // Each runs ${CREATE_FILE_EXE} which will create a file.
  // It also copies that file again using cmake -E.
  // Similar rules have been added to this executable.
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Library/prebuild.txt");
  TestAndRemoveFile(BINARY_DIR "/Library/prelink.txt");
  TestAndRemoveFile(BINARY_DIR "/Library/postbuild.txt");
  TestAndRemoveFile(BINARY_DIR "/Library/postbuild2.txt");
  TestAndRemoveFile(BINARY_DIR "/Executable/prebuild.txt");
  TestAndRemoveFile(BINARY_DIR "/Executable/prelink.txt");
  TestAndRemoveFile(BINARY_DIR "/Executable/postbuild.txt");
  TestAndRemoveFile(BINARY_DIR "/Executable/postbuild2.txt");

  // ----------------------------------------------------------------------
  // A custom target has been created (see Library/).
  // It runs ${CREATE_FILE_EXE} which will create a file.
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Library/custom_target1.txt");

  // ----------------------------------------------------------------------
  // A directory has been created.

  TestDir(BINARY_DIR "/make_dir");

  // ----------------------------------------------------------------------
  // Test OUTPUT_REQUIRED_FILES
  // The files required by 'complex' have been output to a file.
  // The contents of this file is not tested (absolute paths).
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Executable/Temp/complex-required.txt");

  // ----------------------------------------------------------------------
  // Test FIND_LIBRARY

#ifndef FIND_DUMMY_LIB
  cmFailed("the CONFIGURE_FILE command is broken, "
         "FIND_DUMMY_LIB is not defined.");
#else
  if(strstr(FIND_DUMMY_LIB, "dummylib") == NULL)
    {
    cmFailed("the FIND_LIBRARY or CONFIGURE_FILE command is broken, "
           "FIND_DUMMY_LIB == ", FIND_DUMMY_LIB);
    }
  else
    {
    cmPassed("FIND_DUMMY_LIB == ", FIND_DUMMY_LIB);
    }
#endif

  // ----------------------------------------------------------------------
  // Test SET_SOURCE_FILES_PROPERTIES

#ifndef FILE_HAS_EXTRA_COMPILE_FLAGS
  cmFailed("SET_SOURCE_FILES_PROPERTIES failed at setting FILE_HAS_EXTRA_COMPILE_FLAGS flag");
#else
  cmPassed("SET_SOURCE_FILES_PROPERTIES succeeded in setting FILE_HAS_EXTRA_COMPILE_FLAGS flag");
#endif

#ifndef FILE_HAS_ABSTRACT
  cmFailed("SET_SOURCE_FILES_PROPERTIES failed at setting ABSTRACT flag");
#else
  cmPassed("SET_SOURCE_FILES_PROPERTIES succeeded in setting ABSTRACT flag");
#endif

#ifndef FILE_HAS_WRAP_EXCLUDE
  cmFailed("FILE_HAS_WRAP_EXCLUDE failed at setting WRAP_EXCLUDE flag");
#else
  cmPassed("FILE_HAS_WRAP_EXCLUDE succeeded in setting WRAP_EXCLUDE flag");
#endif

#ifndef FILE_COMPILE_FLAGS
  cmFailed("the CONFIGURE_FILE command is broken, FILE_COMPILE_FLAGS is not defined.");
#else
  if(strcmp(FILE_COMPILE_FLAGS, "-foo -bar") != 0)
    {
    cmFailed("the SET_SOURCE_FILES_PROPERTIES or CONFIGURE_FILE command is broken. FILE_COMPILE_FLAGS == ", 
             FILE_COMPILE_FLAGS);
    }
  else
    {
    cmPassed("SET_SOURCE_FILES_PROPERTIES succeeded in setting extra flags == ", FILE_COMPILE_FLAGS);
    }
#endif

  // ----------------------------------------------------------------------
  // Test registry (win32)
#if defined(_WIN32) && !defined(__CYGWIN__)
#ifndef REGISTRY_TEST_PATH
  cmFailed("the CONFIGURE_FILE command is broken, REGISTRY_TEST_PATH is not defined.");
#else
  std::cout << "REGISTRY_TEST_PATH == " << REGISTRY_TEST_PATH << "\n";
  if(stricmp(REGISTRY_TEST_PATH, BINARY_DIR "/registry_dir") != 0)
    {
    cmFailed("the 'read registry value' function or CONFIGURE_FILE command is broken. REGISTRY_TEST_PATH == ", 
             REGISTRY_TEST_PATH, " is not " BINARY_DIR "/registry_dir");
    }
  else
    {
    cmPassed("REGISTRY_TEST_PATH == ", REGISTRY_TEST_PATH);
    }
#endif
#endif // defined(_WIN32) && !defined(__CYGWIN__)

  if(strcmp(CMAKE_MINIMUM_REQUIRED_VERSION, "1.3") == 0)
    {
    cmPassed("CMAKE_MINIMUM_REQUIRED_VERSION is set to 1.3");
    }
  else
    {
    cmFailed("CMAKE_MINIMUM_REQUIRED_VERSION is not set to the expected 1.3");
    }

  // ----------------------------------------------------------------------
  // Test REMOVE command
  if (strcmp("a;b;d",REMOVE_STRING) == 0)
    {
    cmPassed("REMOVE is working");
    }
  else
    {
    cmFailed("REMOVE is not working");
    }
  
  // ----------------------------------------------------------------------
  // Test SEPARATE_ARGUMENTS
  if(strcmp("a;b;c", TEST_SEP) == 0)
    {
    cmPassed("SEPARATE_ARGUMENTS is working");
    }
  else
    {
    cmFailed("SEPARATE_ARGUMENTS is not working");
    }
  
  // ----------------------------------------------------------------------
  // Test Escape Quotes
  if(strcmp("\"hello world\"", STRING_WITH_QUOTES) == 0)
    {
    cmPassed("ESCAPE_QUOTES is working");
    }
  else
    {
    cmFailed("ESCAPE_QUOTES is not working");
    }
    
  
  // ----------------------------------------------------------------------
  // Test cmSystemTools::UpperCase
  std::string str = "abc";
  std::string strupper = "ABC";
  if(cmSystemTools::UpperCase(str) == strupper)
    {
    cmPassed("cmSystemTools::UpperCase is working");
    }
  else
    {
    cmFailed("cmSystemTools::UpperCase is working");
    }    
  
  // ----------------------------------------------------------------------
  // Test if IF command inside a FOREACH works.
#if defined(IF_INSIDE_FOREACH_THEN_EXECUTED) && !defined(IF_INSIDE_FOREACH_ELSE_EXECUTED)
  cmPassed("IF inside a FOREACH block works");
#else
  cmFailed("IF inside a FOREACH block is broken");
#endif

#if defined(GENERATED_HEADER_INCLUDED)
  cmPassed("Generated header included by non-generated source works.");
#else
  cmFailed("Generated header included by non-generated source failed.");
#endif
  
#ifdef FORCE_TEST
  cmFailed("CMake SET CACHE FORCE");
#else
  cmPassed("CMake SET CACHE FORCE");
#endif

  // first run with shouldFail = true, this will
  // run with A B C as set by the CMakeList.txt file.
  if(!TestLibraryOrder(true))
    {
    cmPassed("CMake cmOrderLinkDirectories failed when it should.");
    }
  else
    {
    cmPassed("CMake cmOrderLinkDirectories failed to fail when given an impossible set of paths.");
    }
  // next run with shouldPass = true, this will 
  // run with B/libA.a removed and should create the order
  // B C A
  if(TestLibraryOrder(false))
    {
    cmPassed("CMake cmOrderLinkDirectories worked.");
    }
  else
    {
    cmFailed("CMake cmOrderLinkDirectories failed.");
    }
  
  // ----------------------------------------------------------------------
  // Summary

  std::cout << "Passed: " << cm_passed << "\n";
  if(cm_failed)
    {
    std::cout << "Failed: " << cm_failed << "\n";
    return cm_failed;
    }
  return 0;
}
