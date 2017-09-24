import os
import glob
from distutils.core import setup, Extension

source_files = glob.glob('librxvm/*.c') + ['pyrxvm/pyrxvm.c']

module1 = Extension('rxvm',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['librxvm'],
                    sources = source_files)

HERE = os.path.abspath(os.path.dirname(__file__))
README = os.path.join(HERE, "pyrxvm/README.rst")

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

with open(README, 'r') as f:
    long_description = f.read()

setup (name = 'rxvm',
       version = '1.0.3',
       description = 'Regular expression engine with super-fast file searching',
       long_description=long_description,
       author = 'Erik Nyquist',
       author_email = 'eknyquist@gmail.com',
       url = 'https://github.com/eriknyquist/librxvm',
       ext_modules = [module1])
