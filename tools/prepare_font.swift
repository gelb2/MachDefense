// Offline atlas for the original QobText font. No font file is bundled.
// WeMade ARMv7 QobText font CFString 0x6aaac -> 0x56c63: TrebuchetMS-Bold.
import Foundation
import CoreText
import CoreGraphics
import ImageIO
import CryptoKit

let output = URL(fileURLWithPath: CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "Resources/Generated", isDirectory: true)
try FileManager.default.createDirectory(at: output, withIntermediateDirectories: true)
let fontName = CommandLine.arguments.count > 2 ? CommandLine.arguments[2] : "TrebuchetMS-Bold"
guard ["TrebuchetMS-Bold", "TrebuchetMS"].contains(fontName) else { fatalError("Unsupported original font") }
let pointSize: CGFloat = 48
let font = CTFontCreateWithName(fontName as CFString, pointSize, nil)
let actualName = CTFontCopyPostScriptName(font) as String
guard actualName == fontName else { fatalError("Original font unavailable: \(actualName)") }
let columns = 16, cellWidth = 80, cellHeight = 80, rows = 6
let width = columns * cellWidth, height = rows * cellHeight
let colorSpace = CGColorSpaceCreateDeviceRGB()
guard let context = CGContext(data: nil, width: width, height: height, bitsPerComponent: 8,
                              bytesPerRow: width * 4, space: colorSpace,
                              bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue) else { fatalError("Bitmap allocation failed") }
context.setFillColor(red: 1, green: 1, blue: 1, alpha: 1)
context.setShouldAntialias(true)
context.setAllowsFontSmoothing(false)
context.setShouldSmoothFonts(false)
let ascent = CTFontGetAscent(font), descent = CTFontGetDescent(font), leading = CTFontGetLeading(font)
var entries: [[String: Any]] = []
var tsv = "Code\tX\tY\tWidth\tHeight\tAdvance\tBearingX\tBearingY\n"
for code in 32...126 {
    var character = UniChar(code), glyph = CGGlyph()
    guard CTFontGetGlyphsForCharacters(font, &character, &glyph, 1) else { fatalError("Missing glyph \(code)") }
    var bounds = CGRect.zero, advance = CGSize.zero
    CTFontGetBoundingRectsForGlyphs(font, .horizontal, &glyph, &bounds, 1)
    CTFontGetAdvancesForGlyphs(font, .horizontal, &glyph, &advance, 1)
    let slot = code - 32
    let x = (slot % columns) * cellWidth
    let y = (slot / columns) * cellHeight
    // Full cells share a common baseline. Metrics describe glyph ink bounds;
    // a consumer can also draw a full 80x80 cell with 48-point scaling.
    let baselineFromTop = CGFloat(8) + ascent
    var position = CGPoint(x: CGFloat(x + 8), y: CGFloat(height - y) - baselineFromTop)
    CTFontDrawGlyphs(font, &glyph, &position, 1, context)
    let entry: [String: Any] = ["code": code, "character": String(UnicodeScalar(code)!),
        "rect_xywh": [x, y, cellWidth, cellHeight], "advance": advance.width,
        "bearing_x": bounds.minX, "bearing_y": bounds.maxY,
        "ink_bounds": [bounds.minX, bounds.minY, bounds.width, bounds.height]]
    entries.append(entry)
    tsv += "\(code)\t\(x)\t\(y)\t\(cellWidth)\t\(cellHeight)\t\(advance.width)\t\(bounds.minX)\t\(bounds.maxY)\n"
}
guard let image = context.makeImage(), let destination = CGImageDestinationCreateWithURL(
    output.appendingPathComponent("\(fontName)-48.png") as CFURL, "public.png" as CFString, 1, nil) else {
    fatalError("PNG destination unavailable")
}
CGImageDestinationAddImage(destination, image, nil)
guard CGImageDestinationFinalize(destination) else { fatalError("PNG write failed") }
let pngData = try Data(contentsOf: output.appendingPathComponent("\(fontName)-48.png"))
let pngHash = SHA256.hash(data: pngData).map { String(format: "%02x", $0) }.joined()
let fontURL = CTFontCopyAttribute(font, kCTFontURLAttribute) as? URL
let fontHash = try fontURL.map { try SHA256.hash(data: Data(contentsOf: $0)).map { String(format: "%02x", $0) }.joined() }
let metadata: [String: Any] = ["font": actualName, "font_size": pointSize, "ascent": ascent,
    "descent": descent, "leading": leading, "baseline_from_cell_top": 8 + ascent,
    "left_padding": 8, "columns": columns, "rows": rows, "cell_width": cellWidth,
    "cell_height": cellHeight, "width": width, "height": height,
    "image": "\(fontName)-48.png", "image_sha256": pngHash,
    "font_file_sha256": fontHash ?? "unavailable",
    "original_font_reference": fontName == "TrebuchetMS-Bold" ? "WeMade 1.0.3 ARMv7 CFString 0x6aaac -> 0x56c63; QobText" : "WeMade 1.0.3 ARMv7 CFString 0x6c00c; DlgUpgradeItem",
    "generation": "Offline CoreText glyph rasterization; no runtime text images; installed font file not copied",
    "glyphs": entries]
try JSONSerialization.data(withJSONObject: metadata, options: [.prettyPrinted, .sortedKeys]).write(to: output.appendingPathComponent("\(fontName)-48.json"))
try tsv.write(to: output.appendingPathComponent("\(fontName)-48.tsv"), atomically: true, encoding: .utf8)
print("Prepared \(entries.count) ASCII glyphs: \(actualName), \(width)x\(height)")
