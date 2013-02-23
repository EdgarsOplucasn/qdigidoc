<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>${MACOSX_BUNDLE_EXECUTABLE_NAME}</string>
	<key>CFBundleIconFile</key>
	<string>${MACOSX_BUNDLE_ICON_FILE}</string>
	<key>CFBundleIdentifier</key>
	<string>${MACOSX_BUNDLE_GUI_IDENTIFIER}</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>${MACOSX_BUNDLE_BUNDLE_NAME}</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>${MACOSX_BUNDLE_SHORT_VERSION_STRING}</string>
	<key>CFBundleVersion</key>
	<string>${MACOSX_BUNDLE_BUNDLE_VERSION}</string>
	<key>NSHumanReadableCopyright</key>
	<string>${MACOSX_BUNDLE_COPYRIGHT}</string>
	<key>LSHasLocalizedDisplayName</key>
	<true/>
	<key>LSApplicationCategoryType</key>
	<string>public.app-category.productivity</string>
	<key>LSMinimumSystemVersion</key>
	<string>10.6.0</string>
	<key>CFBundleDocumentTypes</key>
	<array>
		<dict>
			<key>CFBundleTypeExtensions</key>
			<array>
				<string>bdoc</string>
				<string>asice</string>
				<string>sce</string>
			</array>
			<key>CFBundleTypeIconFile</key>
			<string>bdoc.icns</string>
			<key>CFBundleTypeMIMETypes</key>
			<array>
				<string>application/vnd.bdoc-1.0</string>
				<string>application/vnd.etsi.asic-e+zip</string>
			</array>
			<key>CFBundleTypeName</key>
			<string>BDOC – XAdES signatures container</string>
			<key>CFBundleTypeRole</key>
			<string>Editor</string>
			<key>LSHandlerRank</key>
			<string>Owner</string>
			<key>LSItemContentTypes</key>
			<array>
				<string>ee.ria.bdoc</string>
			</array>
		</dict>
		<dict>
			<key>CFBundleTypeExtensions</key>
			<array>
				<string>ddoc</string>
			</array>
			<key>CFBundleTypeIconFile</key>
			<string>ddoc.icns</string>
			<key>CFBundleTypeMIMETypes</key>
			<array>
				<string>application/x-ddoc</string>
			</array>
			<key>CFBundleTypeName</key>
			<string>DDOC – XAdES signatures container</string>
			<key>CFBundleTypeRole</key>
			<string>Editor</string>
			<key>LSHandlerRank</key>
			<string>Owner</string>
			<key>LSItemContentTypes</key>
			<array>
				<string>ee.ria.ddoc</string>
			</array>
		</dict>
		<dict>
			<key>CFBundleTypeExtensions</key>
			<array>
				<string>p12</string>
				<string>p12d</string>
				<string>pfx</string>
			</array>
			<key>CFBundleTypeIconFile</key>
			<string>p12d.icns</string>
			<key>CFBundleTypeMIMETypes</key>
			<array>
				<string>application/x-pkcs12</string>
			</array>
			<key>CFBundleTypeName</key>
			<string>PKCS #12 archive of certificate and private key</string>
			<key>CFBundleTypeRole</key>
			<string>Viewer</string>
			<key>LSHandlerRank</key>
			<string>Default</string>
			<key>LSItemContentTypes</key>
			<array>
				<string>ee.ria.p12d</string>
			</array>
		</dict>
		<dict>
			<key>CFBundleTypeName</key>
			<string>All files</string>
			<key>CFBundleTypeOSTypes</key>
			<array>
				<string>****</string>
			</array>
			<key>CFBundleTypeRole</key>
			<string>Viewer</string>
		</dict>
	</array>
	<key>NSServices</key>
	<array>
		<dict>
			<key>NSMenuItem</key>
			<dict>
				<key>default</key>
				<string>Sign with DigiDoc3 Client</string>
			</dict>
			<key>NSMessage</key>
			<string>openFile</string>
			<key>NSPortName</key>
			<string>${MACOSX_BUNDLE_EXECUTABLE_NAME}</string>
			<key>NSRequiredContext</key>
			<dict>
				<key>NSTextContent</key>
				<string>FilePath</string>
			</dict>
			<key>NSSendTypes</key>
			<array>
				<string>public.url</string>
			</array>
		</dict>
	</array>
	<key>UTExportedTypeDeclarations</key>
	<array>
		<dict>
			<key>UTTypeConformsTo</key>
			<array>
				<string>public.archive</string>
				<string>public.data</string>
			</array>
			<key>UTTypeDescription</key>
			<string>BDOC – XAdES signatures container</string>
			<key>UTTypeIconFile</key>
			<string>bdoc.icns</string>
			<key>UTTypeIdentifier</key>
			<string>ee.ria.bdoc</string>
			<key>UTTypeTagSpecification</key>
			<dict>
				<key>com.apple.ostype</key>
				<string>BDOC</string>
				<key>public.filename-extension</key>
				<string>bdoc</string>
				<key>public.mime-type</key>
				<array>
					<string>application/vnd.bdoc-1.0</string>
					<string>application/vnd.etsi.asic-e+zip</string>
				</array>
			</dict>
		</dict>
		<dict>
			<key>UTTypeConformsTo</key>
			<array>
				<string>public.xml</string>
				<string>public.data</string>
			</array>
			<key>UTTypeDescription</key>
			<string>DDOC – XAdES signatures container</string>
			<key>UTTypeIconFile</key>
			<string>ddoc.icns</string>
			<key>UTTypeIdentifier</key>
			<string>ee.ria.ddoc</string>
			<key>UTTypeTagSpecification</key>
			<dict>
				<key>com.apple.ostype</key>
				<string>DDOC</string>
				<key>public.filename-extension</key>
				<string>ddoc</string>
				<key>public.mime-type</key>
				<array>
					<string>application/x-ddoc</string>
				</array>
			</dict>
		</dict>
		<dict>
			<key>UTTypeConformsTo</key>
			<array>
				<string>public.data</string>
			</array>
			<key>UTTypeDescription</key>
			<string>PKCS #12 archive of certificate and private key</string>
			<key>UTTypeIconFile</key>
			<string>p12d.icns</string>
			<key>UTTypeIdentifier</key>
			<string>ee.ria.p12d</string>
			<key>UTTypeTagSpecification</key>
			<dict>
				<key>public.filename-extension</key>
				<array>
					<string>p12</string>
					<string>p12d</string>
					<string>pfx</string>
				</array>
				<key>public.mime-type</key>
				<array>
					<string>application/x-pkcs12</string>
				</array>
			</dict>
		</dict>
	</array>
</dict>
</plist>
