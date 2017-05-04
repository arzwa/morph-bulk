from distutils.core import setup

setup(
    name='morph_bulk',
    version='1.0',
    packages=['morph_bulk'],
    url='',
    license='',
    author='Arthur Zwanepoel',
    author_email='arzwa@psb.vib-ugent.be',
    description='MORPH bulk CLI',
    py_modules=['morphbulk'],
    include_package_data=True,
    install_requires=[
        'click',
    ],
    entry_points='''
        [console_scripts]
        morphbulk=morphbulk:cli
    ''',
)
