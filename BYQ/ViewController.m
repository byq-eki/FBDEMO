//
//  ViewController.m
//  BYQ
//
//  Created by 卞以其 on 2018/12/25.
//  Copyright © 2018年 卞以其. All rights reserved.
//

#import "ViewController.h"
#import "FBSDK.h"
#import <FBSDKCoreKit/FBSDKCoreKit.h>

@interface ViewController ()

@end

@implementation ViewController

bool isLogin = false;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    isLogin = [[FBSDK sharedInstance] checkLogin];
    self.mLoginBTN.backgroundColor = [UIColor lightGrayColor];
    self.mShareImg.backgroundColor = [UIColor lightGrayColor];
    self.mStatusLab.textAlignment = NSTextAlignmentCenter;
    [self setState];
    [FBSDKProfile enableUpdatesOnAccessTokenChange:YES];
    [[NSNotificationCenter defaultCenter] addObserverForName:FBSDKProfileDidChangeNotification
                                                      object:nil
                                                       queue:[NSOperationQueue mainQueue]
                                                  usingBlock:
     ^(NSNotification *notification) {
         isLogin = [FBSDKProfile currentProfile];
         [self setState];
     }];
}
- (IBAction)onLoginClick:(id)sender {
    if (isLogin) {
        [[FBSDK sharedInstance] fbLogout];
    } else {
        [[FBSDK sharedInstance] fbLogin: self];
    }
}

- (IBAction)onShareImg:(id)sender {
    [[FBSDK sharedInstance] shareImg: self];
    UIImage *image = [UIImage imageNamed:@"pikaqiu.png"];
    self.mImage.image = image;
    self.mImage.contentMode = UIViewContentModeScaleAspectFit;
}

- (IBAction)onShareLink:(id)sender {
    [[FBSDK sharedInstance] shareLink:self];
}


- (void) setState{
    if (isLogin) {
        [self.mLoginBTN setTitle:@"LOG OUT" forState:UIControlStateNormal];
        self.mStatusLab.text = [[[FBSDKProfile currentProfile] name] stringByAppendingString:@"  has logged in"];
    } else {
        [self.mLoginBTN setTitle:@"LOG IN" forState:UIControlStateNormal];
        self.mStatusLab.text = @"not logged in";
    }
}

@end
