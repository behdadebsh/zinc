'''
Created on October 11, 2013

@author: Alan Wu
'''
import unittest

from opencmiss.zinc.context import Context
from opencmiss.zinc import status

class ImagefilterMeanTestsCase(unittest.TestCase):

    def setUp(self):
        self.context = Context("ImagefilterMeanTest")
        root_region = self.context.getDefaultRegion()
        self.field_module = root_region.getFieldmodule()

    def tearDown(self):
        del self.field_module
        del self.context

    def testFieldImagefilterMeanCreate(self):
        self.assertRaises(TypeError, self.field_module.createFieldImagefilterMean, [1])
        imageField = self.field_module.createFieldImage()
        si = imageField.createStreaminformation()
        sr = si.createStreamresourceFile('resource/testimage_gray.jpg')
        imageField.read(si)
        f = self.field_module.createFieldImagefilterMean(imageField, [3])
        self.assertTrue(f.isValid())

def suite():
    #import ImportTestCase
    tests = unittest.TestSuite()
    tests.addTests(unittest.TestLoader().loadTestsFromTestCase(ImagefilterMeanTestsCase))
    return tests

if __name__ == '__main__':
    unittest.TextTestRunner().run(suite())
