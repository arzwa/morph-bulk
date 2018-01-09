from distutils.core import setup

setup(
    name='morph-bulk',
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
        'click==6.7',
        'rdflib==4.2.2',
        'coloredlogs==7.3',
        'numpy==1.13.1',
        'pandas==0.20.3',
        'ruamel.yaml==0.14.9',
        'scipy==0.19.1',
        'godb'
    ],
    entry_points='''
        [console_scripts]
        morph-bulk=morphbulk:cli
    ''',
)
