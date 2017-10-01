import os
import glob
import wheel
import unittest
from setuptools import setup, Extension, Command

PYRXVM_DIR = 'pyrxvm'
source_files = glob.glob('librxvm/*.c') + ['pyrxvm/pyrxvm.c']

module1 = Extension('rxvm',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['librxvm'],
                    sources = source_files)

HERE = os.path.abspath(os.path.dirname(__file__))
README = os.path.join(HERE, os.path.join(PYRXVM_DIR, "README.rst"))

classifiers = [
    'License :: OSI Approved :: Apache Software License',
    'Operating System :: OS Independent',
    'Programming Language :: Python',
    'Natural Language :: English',
    'Intended Audience :: Developers',
    'Intended Audience :: Science/Research',
    'Intended Audience :: Education',
    'Intended Audience :: Information Technology',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 2.7',
]

class RunPyRXVMTests(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        suite = unittest.TestLoader().discover("pyrxvm/test")
        t = unittest.TextTestRunner(verbosity=2)
        t.run(suite)

with open(README, 'r') as f:
    long_description = f.read()

setup (name = 'rxvm',
       version = '1.0.3',
       description = 'Regular expression engine with super-fast file searching',
       long_description=long_description,
       author = 'Erik Nyquist',
       author_email = 'eknyquist@gmail.com',
       url = 'https://github.com/eriknyquist/librxvm',
       package_dir = {'rxvm': PYRXVM_DIR},
       cmdclass={'test': RunPyRXVMTests},
       ext_modules = [module1])
