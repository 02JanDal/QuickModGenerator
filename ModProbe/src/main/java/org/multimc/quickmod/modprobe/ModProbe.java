package org.multimc.quickmod.modprobe;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

import org.reflections.Reflections;
import org.reflections.scanners.SubTypesScanner;
import org.reflections.util.ClasspathHelper;
import org.reflections.util.ConfigurationBuilder;
import org.scannotation.AnnotationDB;
import org.json.*;

import cpw.mods.fml.common.Mod;
import cpw.mods.fml.common.ModMetadata;
import cpw.mods.fml.common.Mod.CustomProperty;
import cpw.mods.fml.common.ModContainer;
import cpw.mods.fml.common.versioning.ArtifactVersion;

public class ModProbe {
	public static void main(String[] args) throws IOException {
		URL forge = new File(args[0]).toURI().toURL();
		JSONStringer writer = new JSONStringer();
		writer.object();
		for (int i = 1; i < args.length; ++i) {
			try {
				writer.key(args[i]).array();
				URL mod = new File(args[i]).toURI().toURL();
				URLClassLoader loader = new URLClassLoader(new URL[]{forge, mod});
				readAtMod(writer, loader, new URL[]{mod});
				// TODO fix for coremods
				//readModContainers(writer, loader);
				loader.close();
			} catch (IOException e) {
				e.printStackTrace();
			} catch (Exception e) {
			} finally {
				writer.endArray();
			}
		}
		writer.endObject();
		System.out.println(writer.toString());
	}
	
	private static void readAtMod(JSONStringer json, URLClassLoader loader, URL[] urls) throws Exception
	{
		AnnotationDB db = new AnnotationDB();
		db.setScanClassAnnotations(true);
		db.setScanFieldAnnotations(false);
		db.setScanMethodAnnotations(false);
		db.setScanParameterAnnotations(false);
		try {
			db.scanArchives(urls);
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}
		Set<String> modClasses = db.getAnnotationIndex().get(
				"cpw.mods.fml.common.Mod");
		if (modClasses == null)
		{
			throw new Exception("No @Mod classes");
		}
		Class<Mod> atmod = Mod.class;
		Iterator<String> modIt = modClasses.iterator();
		while (modIt.hasNext()) {
			try {
				Class<?> clazz = loader.loadClass(modIt.next());
				final String pkg = clazz.getPackage().getName();
				Mod an = clazz.getAnnotation(atmod);
				CustomProperty[] customProperties = an.customProperties();
				HashMap<String, String> map = new HashMap<String, String>();
				for (int cp = 0; cp < customProperties.length; ++cp)
				{
					map.put(customProperties[cp].k(), customProperties[cp].v());
				}
				toJson(json,
						pkg,
						an.modid(),
						an.name(),
						an.version(),
						an.dependencies(),
						an.acceptedMinecraftVersions(),
						an.acceptableRemoteVersions(),
						an.bukkitPlugin(),
						an.certificateFingerprint(),
						an.modLanguage(),
						map);
			} catch (ClassNotFoundException e) {
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
			}
		}
	}
	
	private static void readModContainers(JSONStringer json, URLClassLoader loader) throws InstantiationException, IllegalAccessException
	{
		Reflections reflections = new Reflections(
				new ConfigurationBuilder()
				.addClassLoader(loader)
				.addScanners(new SubTypesScanner())
				.addUrls(ClasspathHelper.forClassLoader(loader))
				);
		Set<Class<? extends ModContainer> > containers = reflections.getSubTypesOf(ModContainer.class);
		Iterator<Class<? extends ModContainer> > it = containers.iterator();
		while (it.hasNext())
		{
			readModContainer(json, reflections, it.next(), new String());
		}
	}
	
	private static void readModContainer(JSONStringer json, Reflections reflections, Class<? extends ModContainer> clazz, String level) throws IllegalAccessException
	{
		System.out.println(level + clazz.getCanonicalName());
		Set<?> containers = reflections.getSubTypesOf(clazz);
		Iterator<Class<? extends ModContainer> > it = (Iterator<Class<? extends ModContainer>>) containers.iterator();
		while (it.hasNext())
		{
			readModContainer(json, reflections, it.next(), level + " ");
		}
		try {
			if (!clazz.getPackage().getName().startsWith("cpw.mods."))
			{
				System.out.println("->" + level + clazz.getCanonicalName());
				ModContainer container = (ModContainer) clazz.newInstance();
				ModMetadata meta = container.getMetadata();
				toJson(json,
						clazz.getClass().getPackage().getName(),
						meta);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private static void toJson(JSONStringer json, String pkg, ModMetadata meta)
	{
		String deps = new String();
		Iterator<ArtifactVersion> it = meta.dependencies.iterator();
		while (it.hasNext())
		{
			ArtifactVersion v = it.next();
			deps += v.toString() + ',';
		}
		toJson(json,
				pkg,
				meta.modId,
				meta.name,
				meta.version,
				deps,
				"",
				"",
				"",
				"",
				"",
				new HashMap<String, String>()
				);
	}
	
	private static void toJson(JSONStringer json, String pkg, String modid, String name, String version, String dependencies,
			String acceptedMinecraftVersions, String acceptableRemoteVersions, String bukkitPlugin, String certificateFingerprint,
			String modLanguage, HashMap<String, String> customProperties)
	{
		json.object()
		.key("modid").value(modid)
		.key("name").value(name)
		.key("version").value(version)
		.key("dependencies").value(dependencies)
		.key("acceptedMinecraftVersions").value(acceptedMinecraftVersions)
		.key("acceptableRemoteVersionsMethod").value(acceptableRemoteVersions)
		.key("bukkitPlugin").value(bukkitPlugin)
		.key("certificateFingerprint").value(certificateFingerprint)
		.key("modLanguageMethod").value(modLanguage)
		.key("customProperties").object();
		Iterator<Entry<String, String>> it = customProperties.entrySet().iterator();
		while (it.hasNext())
		{
			Entry<String, String> item = it.next();
			json.key(item.getKey()).value(item.getValue());
		}
		json.endObject()
		.key("package").value(pkg)
		.endObject();
	}
}
