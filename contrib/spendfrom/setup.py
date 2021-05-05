from distutils.core import setup
setup(name='HODLspendfrom',
      version='1.0',
      description='Command-line utility for hodlcash "coin control"',
      author='Gavin Andresen',
      author_email='gavin@hodlcashfoundation.org',
      requires=['jsonrpc'],
      scripts=['spendfrom.py'],
      )
