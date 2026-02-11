#include <Cocoa/Cocoa.h>
#import "model/MinesweeperGame.hpp"

constexpr float cellSize = 40.f;
size_t columns = 24;
size_t rows = 20;
size_t mines = 99;

MinesweeperGame game(columns, rows, mines);

NSColor* colorA = [NSColor colorWithRed:170.f/255.f green:215.f/255.f blue:82.f/255.f alpha:1.0f];
NSColor* colorB = [NSColor colorWithRed:162.f/255.f green:209.f/255.f blue:74.f/255.f alpha:1.0f];
NSColor* colorARevealed = [NSColor colorWithRed:230.f/255.f green:195.f/255.f blue:160.f/255.f alpha:1.0f];
NSColor* colorBRevealed = [NSColor colorWithRed:215.f/255.f green:184.f/255.f blue:153.f/255.f alpha:1.0f];
NSColor* colorFlag = [NSColor colorWithRed:0.8f green:0.f blue:0.f alpha:1.0f];
NSColor* colorMine = [NSColor colorWithRed:0.f green:0.f blue:0.f alpha:1.0f];

@interface DrawView : NSView
@property (strong, nonatomic) NSTimer* gameTimer;
@property (assign, nonatomic) NSTimeInterval startTime;
@property (assign, nonatomic) NSTimeInterval elapsedTime;
@end

@implementation DrawView
- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        [self setWantsLayer:YES];
        self.elapsedTime = 0;
        [self startTimer];
    }
    return self;
}

- (void)startTimer {
    [self stopTimer];
    [self resetTimer];
    self.gameTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                      target:self
                      selector:@selector(updateTimer:)
                      userInfo:nil
                      repeats:YES];

}

- (void)updateTimer:(NSTimer*)timer {
    self.elapsedTime = [NSDate timeIntervalSinceReferenceDate] - self.startTime;
    [self setNeedsDisplay:YES];
}

- (void)stopTimer {
    if (self.gameTimer) [self.gameTimer invalidate];
    self.gameTimer = nil;
}

- (void)resetTimer {
    self.startTime = [NSDate timeIntervalSinceReferenceDate];
    self.elapsedTime = 0;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent*)event {
    NSString* characters = [event characters];
    if ([characters length] > 0) {
        unichar keyChar = [characters characterAtIndex:0];

        switch (keyChar) {
            case 'r':
            case 'R':
                game = MinesweeperGame(columns, rows, mines);
                [self startTimer];
                break;
        }

        [self setNeedsDisplay:YES];
    }
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event {
    return YES;
}

- (void)mouseDown:(NSEvent*)event {
    if (game.getGameState() != GameState::Ongoing) return;

    NSPoint locationInWindow = [event locationInWindow];
    NSPoint releasePosition = [self convertPoint:locationInWindow fromView:nil];

    GameState state = game.reveal((int)(releasePosition.x / cellSize), (int)(releasePosition.y / cellSize));
    if (state == GameState::Lost || state == GameState::Won) {
        [self stopTimer];
    }

    [self setNeedsDisplay:YES];
}

- (void)rightMouseDown:(NSEvent*)event {
    if (game.getGameState() != GameState::Ongoing) return;

    NSPoint locationInWindow = [event locationInWindow];
    NSPoint releasePosition = [self convertPoint:locationInWindow fromView:nil];

    game.toggleFlag((int)(releasePosition.x / cellSize), (int)(releasePosition.y / cellSize));
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    NSRect bounds = [self bounds];

    [[self window] setContentSize:NSMakeSize(columns * cellSize, rows * cellSize + 30.f)];

    NSString* timerString = [NSString stringWithFormat:@"Time: %.2f", (float)self.elapsedTime];
    NSDictionary* timerAttributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:16.0f],
            NSForegroundColorAttributeName: [NSColor whiteColor]
    };
    [timerString drawAtPoint:NSMakePoint(bounds.size.width / 2.f - 70.f, bounds.size.height - 25.f) withAttributes:timerAttributes];

    NSString* minesString = [NSString stringWithFormat:@"Mines: %d", MAX(0, game.getMinesLeft())];
    NSDictionary* minesAttributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:16.0f],
            NSForegroundColorAttributeName: [NSColor whiteColor]
    };
    [minesString drawAtPoint:NSMakePoint(bounds.size.width / 2.f + 40.f, bounds.size.height - 25.f) withAttributes:minesAttributes];

    for (size_t x = 0; x < columns; x++) {
        float width = cellSize * x;
        for (size_t y = 0; y < rows; y++) {
            float height = cellSize * y;

            auto cellState = game.getCell(x, y);

            NSColor* fillColor = colorA;
            if (cellState.first == CellState::Revealed) fillColor = (x + y) % 2 == 0 ? colorARevealed : colorBRevealed;
            else if (cellState.first == CellState::Mine) fillColor = colorMine;
            else if ((x + y) % 2 == 0) fillColor = colorB;
            [fillColor setFill];

            NSRect cell = NSMakeRect(
                    bounds.origin.x + width,
                    bounds.origin.y + y * cellSize,
                    cellSize, cellSize
            );
            [[NSBezierPath bezierPathWithRect:cell] fill];

            if (cellState.first == CellState::Flagged) {
                [colorFlag setFill];
                NSRect flag = NSMakeRect(
                        bounds.origin.x + width + cellSize * 0.25f,
                        bounds.origin.y + height + cellSize * 0.25f,
                        cellSize * 0.5f, cellSize * 0.5f
                );
                [[NSBezierPath bezierPathWithRect:flag] fill];
            } else if (cellState.first == CellState::Revealed && cellState.second > 0) {
                NSString* numberString = [NSString stringWithFormat:@"%d", cellState.second];
                NSDictionary* attributes = @{
                    NSFontAttributeName: [NSFont systemFontOfSize:cellSize * 0.5f],
                    NSForegroundColorAttributeName: [NSColor blackColor]
                };
                NSSize textSize = [numberString sizeWithAttributes:attributes];
                NSPoint textPosition = NSMakePoint(
                        bounds.origin.x + width + (cellSize - textSize.width) / 2,
                        bounds.origin.y + height + (cellSize - textSize.height) / 2
                );
                [numberString drawAtPoint:textPosition withAttributes:attributes];
            }
        }
    }
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>
@property (strong, nonatomic) NSWindow* window;
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    NSRect frame = NSMakeRect(0, 0, columns * cellSize, rows * cellSize + 30.f);
    self.window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
        backing:NSBackingStoreBuffered
        defer:NO];

    [self.window setTitle:@"Minesweeper"];
    [self.window setDelegate:self];
    [self.window center];

    DrawView *view = [[DrawView alloc] initWithFrame:frame];
    [self.window setContentView:view];

    [self.window orderFrontRegardless];
}

- (void)windowWillClose:(NSNotification*)notification {
    [NSApp terminate:nil];
}
@end

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];

        AppDelegate* delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app activateIgnoringOtherApps:YES];

        [app run];
    }
    return 0;
}