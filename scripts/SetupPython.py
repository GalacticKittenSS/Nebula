import sys
import subprocess
import importlib.util as util

class PythonConfiguration:
    @classmethod
    def Validate(cls):
        if not cls.__ValidatePython():
            return
        
        for packageName in ["requests"]:
            if not cls.__ValidatePackage(packageName):
                return

    @classmethod
    def __ValidatePython(cls, versionMajor = 3, versionMinor = 3):
        if sys.version is not None:
            print("Python version {0:d}.{1:d}.{2:d} detected".format(sys.version_info.major, sys.version_info.minor, sys.version_info.micro))

        if sys.version_info.major < versionMajor or (sys.version_info.major == versionMajor and sys.version_info.minor < versionMinor):
            print("Invalid Python Version, expected {0:d}.{1:d} or higher".format(versionMajor, versionMinor))
            return False

        return True


    @classmethod
    def __ValidatePackage(cls, packageName):
        if util.find_spec(packageName) is None:
            return cls.__InstallPackage(packageName)

        return True

    @classmethod
    def __InstallPackage(cls, packageName):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("Would you like to install Python package '{0:s}'? [Y/N]: ".format(packageName))).lower().strip()[:1]
            if reply == "n":
                return False
                permissionGranted = (reply == 'y')

        print(f"Installing {packageName} module...")
        subprocess.check_call(['python', '-m', 'pip', 'install', packageName])

        return cls.ValidatePackage(packageName)