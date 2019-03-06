//
//  FBSDK.m
//  BYQ
//
//  Created by 卞以其 on 2018/12/25.
//  Copyright © 2018年 卞以其. All rights reserved.
//

#import "FBSDK.h"
#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>
#import <FBSDKShareKit/FBSDKShareKit.h>
#import <FBSDKShareKit/FBSDKSharing.h>
#import "ViewController.h"
#import <UIKit/UIKit.h>

@implementation FBSDK

+ (instancetype)sharedInstance {
    static FBSDK *instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (void)fbLogin: (UIViewController *) controller{
    NSLog(@"fblogin~~~~~~~~~~");
    FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
    [login
     logInWithReadPermissions: @[@"public_profile",@"email"]
     fromViewController:nil
     handler:^(FBSDKLoginManagerLoginResult *result, NSError *error) {
         if (error) {
             NSLog(@"Process error : %@", [error localizedDescription]);
         } else if (result.isCancelled) {
             NSLog(@"Cancelled");
         } else {
             NSLog(@"succeed : token = %@, and userID = %@", result.token.tokenString, result.token.userID);
         }
     }];
}

- (void) fbLogout{
    NSLog(@"fblogout~~~~~~~~~~");
    FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
    [login logOut];
    
    
    NSHTTPCookieStorage* cookies = [NSHTTPCookieStorage sharedHTTPCookieStorage];
    NSArray* facebookCookies = [cookies cookiesForURL:
                                [NSURL URLWithString:@"http://login.facebook.com"]];
    
    for (NSHTTPCookie* cookie in facebookCookies) {
        [cookies deleteCookie:cookie];
    }
    
    NSArray* facebookCookies1 = [cookies cookiesForURL:
                                 [NSURL URLWithString:@"https://login.facebook.com"]];
    
    for (NSHTTPCookie* cookie in facebookCookies1) {
        [cookies deleteCookie:cookie];
    }
    


}

- (BOOL) checkLogin{
    return [FBSDKAccessToken currentAccessToken];
}

- (void) shareImg:(UIViewController *) controller{
    NSLog(@"share image~~~~~~~~~~~~~~~~~~`");
    
//    //获取网络图片地址
    NSURL *url = [NSURL URLWithString: [self URLEncodeString:@"https://ss0.bdstatic.com/94oJfD_bAAcT8t7mm9GUKT-xh_/timg?image&quality=100&size=b4000_4000&sec=1545748278&di=ee02c902ba9b70cbf8ba06ca503827d3&src=http://e.hiphotos.baidu.com/image/pic/item/2cf5e0fe9925bc316045679855df8db1cb137091.jpg"]];
    //将网络地址的NSString类型转化为NSData类型
    NSData *data = [NSData dataWithContentsOfURL:url];
    //image与data的相互转换
    UIImage *image = [UIImage imageWithData:data];
//    UIImage *image = [UIImage imageNamed:@"pikaqiu.png"];
    FBSDKSharePhoto *photo = [[FBSDKSharePhoto alloc] init];
    photo.image = image;
    photo.userGenerated = YES;
    FBSDKSharePhotoContent *content = [[FBSDKSharePhotoContent alloc] init];
    content.photos = @[photo];
//    [FBSDKShareDialog showFromViewController:controller
//                                 withContent:content
//                                    delegate:self];
    FBSDKShareDialog *dialog = [[FBSDKShareDialog alloc] init];
    dialog.shareContent = content;
    dialog.mode = FBSDKShareDialogModeNative;
    if (!dialog.canShow){
        NSLog(@"can't show");
        dialog.mode = FBSDKShareDialogModeWeb;
    }
    dialog.delegate = self;
    [dialog show];
}

- (void) shareLink:(UIViewController *)controller{
    FBSDKShareLinkContent *content = [[FBSDKShareLinkContent alloc] init];
    content.contentURL = [NSURL URLWithString:@"http://evol.papegames.cn"];
//    [FBSDKShareDialog showFromViewController:controller
//                                 withContent:content
//                                    delegate:self];
        FBSDKShareDialog *dialog = [[FBSDKShareDialog alloc] init];
        dialog.shareContent = content;
        dialog.mode = FBSDKShareDialogModeNative;
        if (!dialog.canShow){
            NSLog(@"can't show");
            dialog.mode = FBSDKShareDialogModeWeb;
        }
        dialog.delegate = self;
        [dialog show];
}

-(NSString *) URLEncodeString:(NSString *) str
{
    NSMutableString *tempStr = [NSMutableString stringWithString:str];
    [tempStr replaceOccurrencesOfString:@" " withString:@"+" options:NSCaseInsensitiveSearch range:NSMakeRange(0, [tempStr length])];
    return [[NSString stringWithFormat:@"%@",tempStr] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
}


- (UIImage *)createImageWithColor:(UIColor *)color
{
    //设置长宽
    CGRect rect = CGRectMake(0.0f, 0.0f, 5.0f, 5.0f);
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);
    UIImage *resultImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return resultImage;
}


#pragma mark -FBSHARE DELEGATE
- (void)sharer:(id<FBSDKSharing>)sharer didCompleteWithResults:(NSDictionary *)results{
    NSLog(@"--->有Facebook客户端，成功分享！");
}
- (void)sharer:(id<FBSDKSharing>)sharer didFailWithError:(NSError *)error{
    NSLog(@"--->分享失败！, %@", [error localizedDescription]);
}

- (void)sharerDidCancel:(id<FBSDKSharing>)sharer{
    NSLog(@"--->取消分享！");
}

@end
