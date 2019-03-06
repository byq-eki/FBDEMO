//
//  FBSDK.h
//  BYQ
//
//  Created by 卞以其 on 2018/12/25.
//  Copyright © 2018年 卞以其. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ViewController.h"
#import <FBSDKShareKit/FBSDKShareKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface FBSDK : NSObject<FBSDKSharingDelegate>

+ (instancetype)sharedInstance;

- (void)fbLogin:(UIViewController *) controller;

- (void)fbLogout;

- (BOOL) checkLogin;

- (void)shareImg:(UIViewController *) controller;

- (void)shareLink:(UIViewController *) controller;

@end

NS_ASSUME_NONNULL_END
